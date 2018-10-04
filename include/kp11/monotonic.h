#pragma once

#include "traits.h" // is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits
#include <utility> // forward, exchange

namespace kp11
{
  /// Allocates memory by incrementing a pointer. Deallocation is a no-op.
  /// * `Allocations` is the maximum of successful allocation requests to `Upstream`
  /// * `Upstream` meets the `Resource` concept
  template<std::size_t Allocations, typename Upstream>
  class monotonic
  {
    static_assert(is_resource_v<Upstream>);

  public: // typedefs
    using pointer = typename Upstream::pointer;
    using size_type = typename Upstream::size_type;

  private: // typedefs
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;

  public: // constructors
    /// * `bytes` is the size in bytes of memory to request from `Upstream`
    /// * `alignment` is the alignment in bytes of memory to request from `Upstream`
    /// * `args` are the constructor arguments to `Upstream`
    template<typename... Args>
    monotonic(size_type bytes, size_type alignment, Args &&... args) noexcept :
        bytes(bytes), alignment(alignment), upstream(std::forward<Args>(args)...)
    {
    }
    /// Deleted because a resource is being held and managed.
    monotonic(monotonic const &) = delete;
    /// Deleted because a resource is being held and managed.
    monotonic & operator=(monotonic const &) = delete;
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~monotonic() noexcept
    {
      release();
    }

  public: // modifiers
    /// * Precondition `alignment (from ctor) % alignment == 0`
    /// * Complexity `O(1)`
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(this->alignment % alignment == 0);
      bytes = round_up_to_our_alignment(bytes);
      if (auto ptr = allocate_from_back(bytes))
      {
        return static_cast<pointer>(ptr);
      }
      else if (push_back())
      {
        // this call should not fail as a full buffer should be able to fulfil any request made
        auto ptr = allocate_from_back(bytes);
        assert(ptr != nullptr);
        return static_cast<pointer>(ptr);
      }
      else
      {
        return nullptr;
      }
    }
    /// No-op.
    /// * Complexity `O(0)`
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }

    void release() noexcept
    {
      while (length)
      {
        pop_back();
      }
    }

  private: // allocate helpers
    size_type round_up_to_our_alignment(size_type bytes) const noexcept
    {
      return bytes == 0 ? alignment : (bytes / alignment + (bytes % alignment != 0)) * alignment;
    }
    /// * Precondition `bytes % alignment == 0`
    byte_pointer allocate_from_back(size_type bytes) noexcept
    {
      assert(bytes % alignment == 0);
      if (auto space = static_cast<size_type>(last - first); bytes <= space)
      {
        return std::exchange(first, first + bytes);
      }
      return nullptr;
    }

  private: // modifiers
    /// Allocates a new block of memory from `Upstream`.
    bool push_back() noexcept
    {
      if (length != Allocations)
      {
        if (auto ptr = upstream.allocate(bytes, alignment))
        {
          ptrs[length++] = static_cast<byte_pointer>(ptr);
          first = ptrs[length - 1];
          last = first + bytes;
          return true;
        }
      }
      return false;
    }
    /// Deallocates allocated memory back to `Upstream`.
    void pop_back() noexcept
    {
      assert(length);
      upstream.deallocate(static_cast<pointer>(ptrs[--length]), bytes, alignment);
      first = length ? ptrs[length - 1] : nullptr;
      last = first;
    }

  public: // observers
    pointer operator[](pointer ptr) const noexcept
    {
      for (std::size_t i = 0; i < length; ++i)
      {
        if (std::less_equal<pointer>()(static_cast<pointer>(ptrs[i]), ptr) &&
            std::less<pointer>()(ptr, static_cast<pointer>(ptrs[i] + bytes)))
        {
          return static_cast<pointer>(ptrs[i]);
        }
      }
      return nullptr;
    }

  private: // variables
    /// Size in bytes of memory to allocate from `Upstream`.
    size_type const bytes;
    /// Size in bytes of alignment of memory to allocate from `Upstream`.
    size_type const alignment;
    /// Current position of beginning of allocatable memory.
    byte_pointer first = nullptr;
    /// End of allocatable memory.
    byte_pointer last = nullptr;
    /// Number of allocations from `Upstream`.
    std::size_t length = 0;
    /// Holds pointers to memory allocated by `Upstream`
    byte_pointer ptrs[Allocations];
    Upstream upstream;
  };
}