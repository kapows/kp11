#pragma once

#include "detail/static_vector.h" // static_vector
#include "traits.h" // is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits
#include <utility> // forward, exchange

namespace kp11
{
  /// @brief Advance a pointer through single allocations from `Upstream`. Deallocation is a no-op.
  ///
  /// @tparam ChunkSize Size in bytes of a request to `Upstream`.
  /// @tparam ChunkAlignment Alignment in bytes of a request to `Upstream` and alignment of blocks
  /// and the block size.
  /// @tparam MaxChunks Maximum number of concurrent allocations from `Upstream`.
  /// @tparam Upstream Meets the `Resource` concept.
  template<std::size_t ChunkSize,
    std::size_t ChunkAlignment,
    std::size_t MaxChunks,
    typename Upstream>
  class monotonic
  {
    static_assert(is_resource_v<Upstream>);
    static_assert(ChunkSize % ChunkAlignment == 0);

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
    static constexpr auto block_size = ChunkAlignment;

  private: // typedefs
    /// Byte pointer for arithmetic purposes.
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;

  public: // constructors
    /// Defined because other constructors are defined.
    monotonic() = default;
    /// Deleted because a resource is being held and managed.
    monotonic(monotonic const &) = delete;
    /// Defined because the destructor is defined. `x` is left is a valid but unspecified state.
    monotonic(monotonic && x) noexcept :
        first(x.first), last(x.last), ptrs(std::move(x.ptrs)), upstream(std::move(x.upstream))
    {
      x.ptrs.clear();
    }
    /// Deleted because a resource is being held and managed.
    monotonic & operator=(monotonic const &) = delete;
    /// Defined because the destructor is defined. `x` is left is a valid but unspecified state.
    monotonic & operator=(monotonic && x) noexcept
    {
      if (this != &x)
      {
        first = x.first;
        last = x.last;
        ptrs = std::move(x.ptrs);
        upstream = std::move(x.upstream);

        x.ptrs.clear();
      }
      return *this;
    }
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~monotonic() noexcept
    {
      release();
    }

  public: // capacity
    static constexpr size_type max_size() noexcept
    {
      return chunk_size;
    }

  public: // modifiers
    /// Try to allocate from the latest memory block. Otherwise try to allocate a new memory block
    /// from `Upstream` and allocates from this new memory block.
    /// * Complexity `O(1)`
    ///
    /// @param size Size in bytes of memory to allocate.
    /// @param alignment Alignment in bytes of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a suitable memory block.
    /// @returns (failure) `nullptr`
    ///
    /// @pre `chunk_alignment % alignment == 0`
    pointer allocate(size_type size, [[maybe_unused]] size_type alignment) noexcept
    {
      assert(chunk_alignment % alignment == 0);
      size = round_up_to_our_alignment(size);
      if (auto ptr = allocate_from_back(size))
      {
        return ptr;
      }
      else if (push_back())
      {
        // This call should not fail as a full buffer should be able to fulfil any request made.
        auto ptr = allocate_from_back(size);
        assert(ptr != nullptr);
        return ptr;
      }
      else
      {
        return nullptr;
      }
    }
    /// No-op.
    /// * Complexity `O(0)`
    void deallocate(pointer, size_type, size_type) noexcept
    {
    }
    /// Deallocate allocated memory back to `Upstream` and clear all metadata.
    void release() noexcept
    {
      for (auto && p : ptrs)
      {
        upstream.deallocate(static_cast<pointer>(p), chunk_size, chunk_alignment);
      }
      ptrs.clear();
      last = first = nullptr;
    }

  private: // allocate helpers
    static constexpr size_type round_up_to_our_alignment(size_type size) noexcept
    {
      return size == 0 ? block_size : (size / block_size + (size % block_size != 0)) * block_size;
    }
    /// @pre `size % block_size == 0`.
    pointer allocate_from_back(size_type size) noexcept
    {
      assert(size % block_size == 0);
      if (auto space = static_cast<size_type>(last - first); size <= space)
      {
        return static_cast<pointer>(std::exchange(first, first + size));
      }
      return nullptr;
    }

  private: // modifiers
    /// Allocate a chunk from `Upstream`. Can fail if max chunks has been reached or if `Upstream`
    /// fails allocation.
    ///
    /// @returns (success) `true`
    /// @returns (failure) `false`
    bool push_back() noexcept
    {
      if (ptrs.size() == ptrs.capacity())
      {
        return false;
      }
      if (auto ptr = upstream.allocate(chunk_size, chunk_alignment))
      {
        first = ptrs.emplace_back(static_cast<byte_pointer>(ptr));
        last = first + chunk_size;
        return true;
      }
      return false;
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
      for (auto && p : ptrs)
      {
        if (std::less_equal<pointer>()(static_cast<pointer>(p), ptr) &&
            std::less<pointer>()(ptr, static_cast<pointer>(p + chunk_size)))
        {
          return static_cast<pointer>(p);
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

  private: // variables
    /// Current position of beginning of allocatable memory.
    byte_pointer first = nullptr;
    /// End of allocatable memory.
    byte_pointer last = nullptr;
    /// Holds pointers to memory allocated by `Upstream`
    kp11::detail::static_vector<byte_pointer, max_chunks> ptrs;
    Upstream upstream;
  };
}