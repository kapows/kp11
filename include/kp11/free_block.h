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
  /// Allocate memory in blocks instead of per byte. Each memory block has an associated `Marker`
  /// which will be used to determine allocated and unallocated blocks.
  ///
  /// @tparam Allocations is the maximum of successful allocation requests to `Upstream`.
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
    /// @param block_size Size in bytes of memory blocks.
    /// @param alignment Alignment of memory blocks.
    /// @param args Constructor arguments to `Upstream`.
    template<typename... Args>
    free_block(size_type block_size, size_type alignment, Args &&... args) noexcept :
        block_size(block_size), alignment(alignment), upstream(std::forward<Args>(args)...)
    {
    }
    /// Deleted because a resource is being held and managed.
    free_block(free_block const &) = delete;
    /// Deleted because a resource is being held and managed.
    free_block & operator=(free_block const &) = delete;
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~free_block() noexcept
    {
      release();
    }

  public: // modifiers
    /// Tries to allocate from existing memory blocks using it's associated `Marker`, if that fails
    /// tries to allocate a new memory block from `Upstream` and allocates from this memory using
    /// it's associated `Marker`.
    /// * Complexity `O(n)`.
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    /// @returns (failure) `nullptr`.
    ///
    /// @pre `alignment (from ctor) % alignment == 0`
    ///
    /// @post (success) (Return value) will not be returned again until it has been `deallocated`.
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(this->alignment % alignment == 0);
      auto const num_blocks = to_marker_size(bytes);
      for (std::size_t i = 0, last = ptrs.size(); i < last; ++i)
      {
        if (auto ptr = allocate_from(i, num_blocks))
        {
          return ptr;
        }
      }
      if (push_back())
      {
        // Allocation here should not fail as a full buffer should be able to fulfil any request.
        auto ptr = allocate_from(ptrs.size() - 1, num_blocks);
        assert(ptr != nullptr);
        return ptr;
      }
      else
      {
        return nullptr;
      }
    }
    /// Finds the memory block that `ptr` points into and resets the associated indexes in it's
    /// associated `Marker`. If `ptr` does not point to a memory block owned by us then we do
    /// nothing. `nullptr` is determined to be a pointer to something that we do not own.
    /// * Complexity `O(n)`.
    ///
    /// @param ptr Pointer to the beginning of a memory block.
    /// @param bytes Size in bytes of the memory block.
    /// @param alignment Alignment in bytes of the memory block.
    ///
    /// @returns (success) `true`.
    /// @returns (failure) `false`.
    ///
    /// @pre If `ptr` points to memory owned by us then `bytes` and `alignment` must be the
    /// corresponding arguments to `allocate`.
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      auto p = static_cast<byte_pointer>(ptr);
      if (auto i = find(p); i != ptrs.max_size())
      {
        markers[i].reset(to_marker_index(i, p), to_marker_size(bytes));
        return true;
      }
      return false;
    }
    /// Deallocates allocated memory back to `Upstream` and destroys it's associated `Marker`.
    void release() noexcept
    {
      for (auto && p : ptrs)
      {
        upstream.deallocate(static_cast<pointer>(p), request_size(), alignment);
      }
      ptrs.clear();
      markers.clear();
    }

  private: // allocate helper
    /// Helper function to make it easier to allocate from each `Marker`.
    pointer allocate_from(std::size_t index, std::size_t num_blocks) noexcept
    {
      if (auto i = markers[index].set(num_blocks); i != Marker::size())
      {
        return static_cast<pointer>(ptrs[index] + static_cast<size_type>(i) * block_size);
      }
      return nullptr;
    }

  public: // observers
    /// Checks whether or not `ptr` points in to memory owned by us.
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

  private: // operator[] helper
    /// Finds the index of the memory block to which `ptr` points. This function makes it easier to
    /// deal with our split ptrs/markers structure since we'll need a common index to access the
    /// associated parts.
    ///
    /// @returns (success) Index of the memory block to which `ptr` points.
    /// @returns (failure) `ptrs.max_size()`.
    std::size_t find(byte_pointer ptr) const noexcept
    {
      for (std::size_t i = 0, last = ptrs.size(); i < last; ++i)
      {
        if (std::less_equal<pointer>()(ptrs[i], ptr) &&
            std::less<pointer>()(ptr, ptrs[i] + request_size()))
        {
          return i;
        }
      }
      return ptrs.max_size();
    }

  private: // modifiers
    /// Allocates a new block of memory from `Upstream` and constructs another `Marker` for it. Can
    /// fail if max allocations has been reached or if `Upstream` fails allocation.
    ///
    /// @returns (success) `true`.
    /// @returns (failure) `false`.
    bool push_back() noexcept
    {
      if (ptrs.size() == ptrs.capacity())
      {
        return false;
      }
      if (auto ptr = upstream.allocate(request_size(), alignment))
      {
        ptrs.emplace_back(static_cast<byte_pointer>(ptr));
        markers.emplace_back();
        return true;
      }
      return false;
    }

  private: // size helper
    /// Size in bytes used in request to `Upstream`.
    size_type request_size() const noexcept
    {
      return block_size * static_cast<size_type>(Marker::size());
    }

  private: // Marker helper functions
    /// Convert from `bytes` to number of blocks to use with `Marker`.
    typename Marker::size_type to_marker_size(size_type bytes) const noexcept
    {
      // 1 block minimum
      // moduloc is required to deal with non BlockSize sizes
      size_type s = bytes == 0 ? 1 : bytes / block_size + (bytes % block_size != 0);
      return static_cast<typename Marker::size_type>(s);
    }
    /// Convert from `byte_pointer` to an index to use with `Marker`.
    typename Marker::size_type to_marker_index(std::size_t index, byte_pointer ptr) const noexcept
    {
      auto p = (ptr - ptrs[index]) / block_size;
      return static_cast<typename Marker::size_type>(p);
    }

  private: // variables
    /// Size in bytes of memory blocks.
    size_type const block_size;
    /// Holds pointers to memory allocated by `Upstream`.
    kp11::detail::static_vector<byte_pointer, Allocations> ptrs;
    /// Holds a `Marker` corresponding to each allocation.
    kp11::detail::static_vector<Marker, Allocations> markers;
    /// Size in bytes of alignment of memory blocks.
    size_type const alignment;
    Upstream upstream;
  };
}