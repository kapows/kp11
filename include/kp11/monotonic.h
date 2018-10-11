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
  /// @brief Advances a pointer through single allocations from `Upstream`. Deallocation is a no-op.
  ///
  /// @tparam Allocations Maximum number of concurrent allocations from `Upstream`.
  /// @tparam Upstream Meets the `Resource` concept.
  template<std::size_t Allocations, typename Upstream>
  class monotonic
  {
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
    /// @param bytes Size in bytes of memory to request from `Upstream`.
    /// @param alignment Alignment in bytes of memory to request from `Upstream` and the size of
    /// each memory block.
    /// @param args Constructor arguments to `Upstream`.
    template<typename... Args>
    monotonic(size_type bytes, size_type alignment, Args &&... args) noexcept :
        bytes(bytes), alignment(alignment), upstream(std::forward<Args>(args)...)
    {
    }
    /// Deleted because a resource is being held and managed.
    monotonic(monotonic const &) = delete;
    /// Defined because the destructor is defined.
    monotonic(monotonic && x) noexcept :
        bytes(x.bytes), alignment(x.alignment), first(x.first), last(x.last),
        ptrs(std::move(x.ptrs)), upstream(std::move(x.upstream))
    {
      x.first = nullptr;
      x.last = nullptr;
      x.ptrs.clear();
    }
    /// Deleted because a resource is being held and managed.
    monotonic & operator=(monotonic const &) = delete;
    /// Defined because the destructor is defined.
    monotonic & operator=(monotonic && x) noexcept
    {
      if (this != &x)
      {
        bytes = x.bytes;
        alignment = x.alignment;
        first = x.first;
        last = x.last;
        ptrs = std::move(x.ptrs);
        upstream = std::move(x.upstream);

        x.first = nullptr;
        x.last = nullptr;
        x.ptrs.clear();
      }
      return *this;
    }
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~monotonic() noexcept
    {
      release();
    }

  public: // modifiers
    /// Tries to allocate from the latest memory block. If that fails then tries to allocate a new
    /// memory block from `Upstream` and allocates from this new memory block.
    /// * Complexity `O(1)`
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    /// @returns (failure) `nullptr`.
    ///
    /// @pre `alignment (from ctor) % alignment == 0`
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(this->alignment % alignment == 0);
      bytes = round_up_to_our_alignment(bytes);
      if (auto ptr = allocate_from_back(bytes))
      {
        return ptr;
      }
      else if (push_back())
      {
        // This call should not fail as a full buffer should be able to fulfil any request made.
        auto ptr = allocate_from_back(bytes);
        assert(ptr != nullptr);
        return ptr;
      }
      else
      {
        return nullptr;
      }
    }
    /// No-op.
    /// * Complexity `O(0)`.
    ///
    /// @param ptr Pointer to the beginning of a memory block.
    /// @param bytes Size in bytes of the memory block.
    /// @param alignment Alignment in bytes of the memory block.
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }
    /// Deallocates allocated memory back to `Upstream`.
    void release() noexcept
    {
      for (auto && p : ptrs)
      {
        upstream.deallocate(static_cast<pointer>(p), bytes, alignment);
      }
      ptrs.clear();
      last = first = nullptr;
    }

  private: // allocate helpers
    size_type round_up_to_our_alignment(size_type bytes) const noexcept
    {
      return bytes == 0 ? alignment : (bytes / alignment + (bytes % alignment != 0)) * alignment;
    }
    /// @pre `bytes % alignment == 0`.
    pointer allocate_from_back(size_type bytes) noexcept
    {
      assert(bytes % alignment == 0);
      if (auto space = static_cast<size_type>(last - first); bytes <= space)
      {
        return static_cast<pointer>(std::exchange(first, first + bytes));
      }
      return nullptr;
    }

  private: // modifiers
    /// Allocates a new block of memory from `Upstream`. Can fail if max allocations has been
    /// reached or if `Upstream` fails allocation.
    ///
    /// @returns (success) `true`.
    /// @returns (failure) `false`.
    bool push_back() noexcept
    {
      if (ptrs.size() == ptrs.capacity())
      {
        return false;
      }
      if (auto ptr = upstream.allocate(bytes, alignment))
      {
        first = ptrs.emplace_back(static_cast<byte_pointer>(ptr));
        last = first + bytes;
        return true;
      }
      return false;
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
      for (auto && p : ptrs)
      {
        if (std::less_equal<pointer>()(static_cast<pointer>(p), ptr) &&
            std::less<pointer>()(ptr, static_cast<pointer>(p + bytes)))
        {
          return static_cast<pointer>(p);
        }
      }
      return nullptr;
    }

  private: // variables
    /// Size in bytes of memory to allocate from `Upstream`.
    size_type bytes;
    /// Size in bytes of alignment of memory to allocate from `Upstream`.
    size_type alignment;
    /// Current position of beginning of allocatable memory.
    byte_pointer first = nullptr;
    /// End of allocatable memory.
    byte_pointer last = nullptr;
    /// Holds pointers to memory allocated by `Upstream`
    kp11::detail::static_vector<byte_pointer, Allocations> ptrs;
    Upstream upstream;
  };
}