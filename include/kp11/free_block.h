#pragma once

#include "detail/static_vector.h" // static_vector
#include "traits.h" // is_marker_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits
#include <utility> // forward

namespace kp11
{
  /// @brief Splits single allocations from `Upstream` into multiple blocks that can be allocated.
  ///
  /// Each memory block allocated from `Upstream` has an associated `Marker` which is used to
  /// determine allocated and unallocated blocks.
  ///
  /// @tparam Allocations Maximum number of concurrent allocations from `Upstream`.
  /// @tparam Marker Meets the `Marker` concept.
  /// @tparam Upstream Meets the `Resource` concept.
  template<std::size_t Allocations, typename Marker, typename Upstream>
  class free_block
  {
    static_assert(is_marker_v<Marker>);
    static_assert(is_resource_v<Upstream>);

  public: // typedefs
    /// Pointer type.
    using pointer = typename Upstream::pointer;
    /// Size type.
    using size_type = typename Upstream::size_type;

  private: // typedefs
    /// Byte pointer for arithmetic purposes.
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;

  public: // constructors
    /// @param chunk_size Size in bytes of memory blocks.
    /// @param chunk_alignment Alignment in bytes of memory blocks.
    /// @param initial_allocations Number of initial allocations to try to make.
    /// @param args Constructor arguments to `Upstream`.
    ///
    /// @pre `chunk_size % Marker::size() == 0`
    /// @pre `chunk_size / Marker::size() % chunk_alignment == 0`
    template<typename... Args>
    free_block(size_type chunk_size,
      size_type chunk_alignment,
      size_type initial_allocations = 0,
      Args &&... args) noexcept :
        block_size(chunk_size / Marker::max_size()),
        chunk_size(chunk_size), chunk_alignment(chunk_alignment),
        upstream(std::forward<Args>(args)...)
    {
      assert(chunk_size % Marker::max_size() == 0);
      assert(chunk_size / Marker::max_size() % chunk_alignment == 0);
      assert(initial_allocations <= Allocations);
      for (size_type i = 0; i != initial_allocations; ++i)
      {
        push_back();
      }
    }
    /// Deleted because a resource is being held and managed.
    free_block(free_block const &) = delete;
    /// Defined because the destructor is defined. `x` is left is a valid but unspecified state.
    free_block(free_block && x) noexcept :
        block_size(x.block_size), chunk_size(x.chunk_size), chunk_alignment(x.chunk_alignment),
        biggests(std::move(x.biggests)), ptrs(std::move(x.ptrs)), markers(std::move(x.markers)),
        upstream(std::move(x.upstream))
    {
      x.ptrs.clear();
    }
    /// Deleted because a resource is being held and managed.
    free_block & operator=(free_block const &) = delete;
    /// Defined because the destructor is defined. `x` is left is a valid but unspecified state.
    free_block & operator=(free_block && x) noexcept
    {
      if (this != &x)
      {
        release();
        block_size = x.block_size;
        chunk_size = x.chunk_size;
        chunk_alignment = x.chunk_alignment;
        biggests = std::move(x.biggests);
        ptrs = std::move(x.ptrs);
        markers = std::move(x.markers);
        upstream = std::move(x.upstream);

        x.ptrs.clear();
      }
      return *this;
    }
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~free_block() noexcept
    {
      release();
    }

  public: // modifiers
    /// Check if existing `Marker`s can allocate the required blocks by checking the corresponding
    /// cached `biggests` value. If any can meet the requirements then allocate using the `Marker`
    /// and update it's value in `biggests`. Otherwise try to allocate a new memory block
    /// from `Upstream` and allocate from the new `Marker`.
    /// * Complexity `O(n)`.
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment in bytes of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    /// @returns (failure) `nullptr`.
    ///
    /// @pre `chunk_alignment (from ctor) % alignment == 0`
    ///
    /// @post (success) (return value) will not be returned again until it has been `deallocated`.
    /// Depends on `Marker`.
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(chunk_alignment % alignment == 0);
      auto const num_blocks = to_marker_size(bytes);
      for (std::size_t i = 0, last = biggests.size(); i < last; ++i)
      {
        if (num_blocks <= biggests[i])
        {
          return allocate_from(i, num_blocks);
        }
      }
      if (push_back())
      {
        // New blocks should be able to fulfil any request.
        assert(num_blocks <= biggests.back());
        return allocate_from(ptrs.size() - 1, num_blocks);
      }
      else
      {
        return nullptr;
      }
    }
    /// Finds the memory block that `ptr` points into and resets the associated indexes in its
    /// corresponding `Marker`, also updates the corresponding `biggests` value. If `ptr` does not
    /// point to a memory block owned here then do nothing. `nullptr` is determined to not be owned.
    /// * Complexity `O(n)`.
    ///
    /// @param ptr Pointer to the beginning of a memory block.
    /// @param bytes Size in bytes of the memory block.
    /// @param alignment Alignment in bytes of the memory block.
    ///
    /// @returns (success) `true`. `ptr` is owned.
    /// @returns (failure) `false`. `ptr` is not owned.
    ///
    /// @pre If `ptr` points to memory owned here then `bytes` and `alignment` must be the
    /// corresponding arguments to `allocate`.
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      auto p = static_cast<byte_pointer>(ptr);
      if (auto i = find(p); i != ptrs.max_size())
      {
        markers[i].reset(to_marker_index(i, p), to_marker_size(bytes));
        biggests[i] = markers[i].biggest();
        return true;
      }
      return false;
    }
    /// Deallocates allocated memory back to `Upstream` and resets the corresponding metadata.
    void release() noexcept
    {
      for (auto && p : ptrs)
      {
        upstream.deallocate(static_cast<pointer>(p), chunk_size, chunk_alignment);
      }
      biggests.clear();
      ptrs.clear();
      markers.clear();
    }

    /// Deallocates the most recently allocated memory back to `Upstream` if their markers have all
    /// vacant spots.
    void shrink_to_fit() noexcept
    {
      // Use `ptrs` here instead of `markers` so that it works with "moved from" objects.
      while (ptrs.size())
      {
        auto & m = markers.back();
        if (m.size() == m.max_size())
        {
          pop_back();
        }
        else
        {
          break;
        }
      }
    }

  private: // allocate helper
    /// Helper function to make it easier to allocate from each `Marker` and update the
    /// corresponding `biggests` value. Function does not return `nullptr`.
    ///
    /// @pre `num_blocks <= biggests[index]`.
    ///
    /// @returns Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    pointer allocate_from(std::size_t index, std::size_t num_blocks) noexcept
    {
      assert(num_blocks <= biggests[index]);
      auto const i = markers[index].set(num_blocks);
      biggests[index] = markers[index].biggest();
      return static_cast<pointer>(ptrs[index] + static_cast<size_type>(i) * block_size);
    }

  public: // observers
    /// Checks whether or not `ptr` points in to memory owned here.
    ///
    /// @param ptr Pointer to memory.
    ///
    /// @returns (success) Pointer to the beginning of the memory block to which `ptr` points.
    /// @returns (failure) `nullptr`.
    pointer operator[](pointer ptr) const noexcept
    {
      if (auto i = find(static_cast<byte_pointer>(ptr)); i != ptrs.max_size())
      {
        return static_cast<pointer>(ptrs[i]);
      }
      return nullptr;
    }
    /// @returns Size of each free block.
    size_type get_block_size() const noexcept
    {
      return block_size;
    }

  public: // accessors
    /// @returns Reference to `Upstream`.
    Upstream & get_upstream() noexcept
    {
      return upstream;
    }
    /// @returns Reference to `Upstream`.
    Upstream const & get_upstream() const noexcept
    {
      return upstream;
    }

  private: // helper
    /// Finds the index of the memory block to which `ptr` points. This function makes it easier to
    /// deal with our split biggests/ptrs/markers structure since we'll need a common index to
    /// access the corresponding parts.
    ///
    /// @returns (success) Index of the memory block to which `ptr` points.
    /// @returns (failure) `ptrs.max_size()`.
    std::size_t find(byte_pointer ptr) const noexcept
    {
      for (std::size_t i = 0, last = ptrs.size(); i < last; ++i)
      {
        if (std::less_equal<pointer>()(ptrs[i], ptr) &&
            std::less<pointer>()(ptr, ptrs[i] + chunk_size))
        {
          return i;
        }
      }
      return ptrs.max_size();
    }

  private: // modifiers
    /// Allocates a new block of memory from `Upstream` and constructs another `Marker` for it and a
    /// `biggest` cached value. Will fail if max allocations has been reached or if `Upstream` fails
    /// allocation.
    ///
    /// @returns (success) `true`.
    /// @returns (failure) `false`.
    bool push_back() noexcept
    {
      if (ptrs.size() == ptrs.capacity())
      {
        return false;
      }
      if (auto ptr = upstream.allocate(chunk_size, chunk_alignment))
      {
        ptrs.emplace_back(static_cast<byte_pointer>(ptr));
        markers.emplace_back();
        biggests.emplace_back(markers.back().biggest());
        return true;
      }
      return false;
    }

    /// Deallocates the most recently allocated block of memory back to `Upstream`.
    ///
    /// @pre `ptrs.empty() == false`
    void pop_back() noexcept
    {
      assert(!ptrs.empty());
      upstream.deallocate(ptrs.back(), chunk_size, chunk_alignment);
      ptrs.pop_back();
      biggests.pop_back();
      markers.pop_back();
    }

  private: // Marker helper functions
    /// Convert from `bytes` to number of blocks.
    typename Marker::size_type to_marker_size(size_type bytes) const noexcept
    {
      // 1 block minimum
      // modulo is required to deal with non block_size sizes
      size_type s = bytes == 0 ? 1 : bytes / block_size + (bytes % block_size != 0);
      return static_cast<typename Marker::size_type>(s);
    }
    /// Convert from `byte_pointer` to an index.
    typename Marker::size_type to_marker_index(std::size_t index, byte_pointer ptr) const noexcept
    {
      auto p = (ptr - ptrs[index]) / block_size;
      return static_cast<typename Marker::size_type>(p);
    }

  private: // variables
    /// Size in bytes of a free block.
    size_type block_size;
    /// Size in bytes of memory allocated from `Upstream`.
    size_type chunk_size;
    /// Alignment in bytes of memory allocated from `Upstream` and of free blocks.
    size_type chunk_alignment;
    /// Holds a biggest size corresponding to each `Marker`.
    kp11::detail::static_vector<typename Marker::size_type, Allocations> biggests;
    /// Holds pointers to memory allocated by `Upstream`.
    kp11::detail::static_vector<byte_pointer, Allocations> ptrs;
    /// Holds a `Marker` corresponding to each allocation.
    kp11::detail::static_vector<Marker, Allocations> markers;
    Upstream upstream;
  };
}