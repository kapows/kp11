#pragma once

#include "detail/static_vector.h" // static_vector
#include "traits.h" // is_marker_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits

namespace kp11
{
  /// @private
  namespace free_block_detail
  {
    /// @private
    template<typename BytePointer,
      typename SizeType,
      typename Marker,
      std::size_t chunk_size,
      std::size_t block_size>
    class resource
    {
    public: // typedefs
      using byte_pointer = BytePointer;
      using size_type = SizeType;

    private: // variables
      byte_pointer ptr;
      Marker marker;

    public: // constructors
      explicit resource(byte_pointer ptr) noexcept : ptr(ptr)
      {
      }

    public: // accessors
      byte_pointer get_ptr() const noexcept
      {
        return ptr;
      }
      Marker const & get_marker() const noexcept
      {
        return marker;
      }

    public: // modifiers
      byte_pointer allocate(size_type size) noexcept
      {
        auto const n = to_blocks(size);
        if (auto i = marker.allocate(n); i != Marker::size())
        {
          return ptr + static_cast<size_type>(block_size * i);
        }
        return nullptr;
      }
      bool deallocate(byte_pointer ptr, size_type size) noexcept
      {
        if (contains(ptr))
        {
          marker.deallocate(to_index(ptr), to_blocks(size));
          return true;
        }
        return false;
      }

    public: // observers
      bool contains(byte_pointer ptr) const noexcept
      {
        return std::less_equal<byte_pointer>()(this->ptr, ptr) &&
               std::less<byte_pointer>()(ptr, this->ptr + static_cast<size_type>(chunk_size));
      }

    private: // helpers
      auto to_index(byte_pointer ptr) const noexcept
      {
        return static_cast<typename Marker::size_type>(
          (ptr - this->ptr) / static_cast<size_type>(block_size));
      }
      static auto to_blocks(size_type size) noexcept
      {
        // size == 0 to deal add 1 when size is 0
        // modulo is required to deal with non block_size sizes
        size_type s = (size == 0) + (size / block_size) + (size % block_size != 0);
        return static_cast<typename Marker::size_type>(s);
      }
    };
  }
  /// @brief Splits single allocations from `Upstream` into multiple blocks that can be allocated.
  ///
  /// Each memory block allocated from `Upstream` has a `Marker` to manage blocks.
  ///
  /// @tparam ChunkSize Size in bytes of request to `Upstream`.
  /// @tparam ChunkAlignment Alignment in bytes of request to `Upstream` and alignment of blocks.
  /// @tparam MaxChunks Maximum number of concurrent allocations from `Upstream`.
  /// @tparam Marker Meets the `Marker` concept.
  /// @tparam Upstream Meets the `Resource` concept.
  template<std::size_t ChunkSize,
    std::size_t ChunkAlignment,
    std::size_t MaxChunks,
    typename Marker,
    typename Upstream>
  class free_block
  {
    static_assert(is_marker_v<Marker>);
    static_assert(is_resource_v<Upstream>);
    static_assert(ChunkSize % ChunkAlignment == 0);
    static_assert(ChunkSize % Marker::size() == 0);
    // Block size must be aligned to chunk alignment.
    static_assert(ChunkSize / Marker::size() % ChunkAlignment == 0);

  public: // typedefs
    /// Pointer type.
    using pointer = typename Upstream::pointer;
    /// Size type.
    using size_type = typename Upstream::size_type;

  public: // constants
    /// Size in bytes of request to `Upstream`.
    static constexpr auto chunk_size = ChunkSize;
    /// Alignment in bytes of request to `Upstream` and alignment of blocks.
    static constexpr auto chunk_alignment = ChunkAlignment;
    /// Maximum number of concurrent allocations from `Upstream`.
    static constexpr auto max_chunks = MaxChunks;
    /// Size in bytes of a free block.
    static constexpr auto block_size = chunk_size / Marker::size();

  private: // typedefs
    /// Byte pointer for arithmetic purposes.
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;
    using resource =
      free_block_detail::resource<byte_pointer, size_type, Marker, chunk_size, block_size>;

  public: // constructors
    /// Defined because other constructors are defined.
    free_block() = default;
    /// Deleted because a resource is being held and managed.
    free_block(free_block const &) = delete;
    /// Defined because the destructor is defined. `x` is left is a valid but unspecified state.
    free_block(free_block && x) noexcept :
        resources(std::move(x.resources)), upstream(std::move(x.upstream))
    {
      x.resources.clear();
    }
    /// Deleted because a resource is being held and managed.
    free_block & operator=(free_block const &) = delete;
    /// Defined because the destructor is defined. `x` is left is a valid but unspecified state.
    free_block & operator=(free_block && x) noexcept
    {
      if (this != &x)
      {
        release();
        resources = std::move(x.resources);
        upstream = std::move(x.upstream);
        x.resources.clear();
      }
      return *this;
    }
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~free_block() noexcept
    {
      release();
    }

  public: // capacity
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return block_size * marker_traits<Marker>::max_size();
    }

  public: // modifiers
    /// Try to allocate from existing allocations. If unsuccessful try to allocate a new memory
    /// block from `Upstream` and allocate from that.
    /// * Complexity `O(n)`
    ///
    /// @param size Size in bytes of memory to allocate.
    /// @param alignment Alignment in bytes of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a suitable memory block.
    /// @returns (failure) `nullptr`
    ///
    /// @pre `chunk_alignment % alignment == 0`
    /// @pre `size <= max_size()`
    ///
    /// @post (success) (return value) will not be returned again until it has been `deallocated`.
    /// Depends on `Marker`.
    pointer allocate(size_type size, [[maybe_unused]] size_type alignment) noexcept
    {
      assert(chunk_alignment % alignment == 0);
      assert(size <= max_size());
      for (auto && r : resources)
      {
        if (auto p = r.allocate(size))
        {
          return static_cast<pointer>(p);
        }
      }
      if (push_back())
      {
        auto p = resources.back().allocate(size);
        // New resources should be able to fulfil any request.
        assert(p != nullptr);
        return static_cast<pointer>(p);
      }
      return nullptr;
    }
    /// If `ptr` points into one of our allocations then deallocate it.
    /// `nullptr` is determined to not be owned.
    /// * Complexity `O(n)`
    ///
    /// @param ptr Pointer to the beginning of a memory block.
    /// @param size Size in bytes of the memory block.
    /// @param alignment Alignment in bytes of the memory block.
    ///
    /// @returns (success) `true`
    /// @returns (failure) `false`
    ///
    /// @pre If `ptr` points into one of our allocations then `size` and `alignment` must be the
    /// corresponding arguments to `allocate`.
    bool deallocate(pointer ptr, size_type size, [[maybe_unused]] size_type alignment) noexcept
    {
      for (auto && r : resources)
      {
        if (r.deallocate(static_cast<byte_pointer>(ptr), size))
        {
          return true;
        }
      }
      return false;
    }
    /// Deallocate allocated memory back to `Upstream` and clear all metadata.
    void release() noexcept
    {
      for (auto && r : resources)
      {
        upstream.deallocate(static_cast<pointer>(r.get_ptr()), chunk_size, chunk_alignment);
      }
      resources.clear();
    }

    /// Deallocate the most recently allocated memory back to `Upstream` if their markers have all
    /// unallocated indexes.
    void shrink_to_fit() noexcept
    {
      while (!resources.empty() && resources.back().get_marker().count() == 0)
      {
        pop_back();
      }
    }

  public: // observers
    /// Check whether or not `ptr` points into an allocation from `Upstream`.
    ///
    /// @param ptr Pointer to memory.
    ///
    /// @returns (success) Pointer to the beginning of the memory block to which `ptr` points.
    /// @returns (failure) `nullptr`
    pointer operator[](pointer ptr) const noexcept
    {
      for (auto && r : resources)
      {
        if (r.contains(static_cast<byte_pointer>(ptr)))
        {
          return static_cast<pointer>(r.get_ptr());
        }
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

  private: // modifiers
    /// Allocate from `Upstream` and construct another resource. Fail if max chunks has been reached
    /// or if `Upstream` fails allocation.
    ///
    /// @returns (success) `true`
    /// @returns (failure) `false`
    bool push_back() noexcept
    {
      if (resources.size() == resources.capacity())
      {
        return false;
      }
      if (auto ptr = static_cast<byte_pointer>(upstream.allocate(chunk_size, chunk_alignment)))
      {
        resources.emplace_back(ptr);
        return true;
      }
      return false;
    }

    /// Deallocate the most recent allocation to `Upstream`.
    ///
    /// @pre `resources.empty() == false`
    void pop_back() noexcept
    {
      assert(!resources.empty());
      upstream.deallocate(
        static_cast<pointer>(resources.back().get_ptr()), chunk_size, chunk_alignment);
      resources.pop_back();
    }

  private: // variables
    kp11::detail::static_vector<resource, max_chunks> resources;
    Upstream upstream;
  };
}