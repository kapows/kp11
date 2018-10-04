#pragma once

#include "traits.h" // is_strategy_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits

namespace kp11
{
  /// Allocates from a buffer inside itself.
  /// Can only allocate one memory block at a time.
  /// * `Pointer` pointer type
  /// * `Size` size type
  /// * `Bytes` is the size of the buffer
  /// * `Alignment` is the alignment of the buffer
  template<typename Pointer, typename SizeType, std::size_t Bytes, std::size_t Alignment>
  class basic_local
  {
  public: // typedefs
    using pointer = Pointer;
    using size_type = SizeType;

  private: // typedefs
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;
    using byte_pointer_traits = std::pointer_traits<byte_pointer>;

  public: // modifiers
    /// * Precondition `alignment (from ctor) % alignment == 0`
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(Alignment % alignment == 0);
      if (!allocated && bytes <= Bytes)
      {
        allocated = true;
        return static_cast<pointer>(buffer_ptr());
      }
      return nullptr;
    }
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (static_cast<byte_pointer>(ptr) == buffer_ptr())
      {
        allocated = false;
        return true;
      }
      return false;
    }

  public: // observers
    pointer operator[](pointer ptr) noexcept
    {
      if (has(static_cast<byte_pointer>(ptr)))
      {
        return static_cast<pointer>(buffer_ptr());
      }
      return nullptr;
    }

  private: // helpers
    /// Check if `ptr` points inside our buffer.
    /// * Returns `true` if `ptr` belongs to us
    /// * Returns `false` on otherwise
    bool has(byte_pointer ptr) noexcept
    {
      if (auto const buf = buffer_ptr();
          std::less_equal<byte_pointer>()(buf, ptr) && std::less<byte_pointer>()(ptr, buf + Bytes))
      {
        return true;
      }
      return false;
    }

  private: // accessors
    /// * Returns `byte_pointer` created from our inner buffer.
    byte_pointer buffer_ptr() noexcept
    {
      return byte_pointer_traits::pointer_to(buffer[0]);
    }

  private: // variables
    bool allocated = false;
    alignas(Alignment) std::byte buffer[Bytes];
  };

  /// Allocates from a buffer inside itself.
  /// Can only allocate one memory block at a time.
  /// * `Bytes` is the size of the buffer
  /// * `Alignment` is the alignment of the buffer
  template<std::size_t Bytes, std::size_t Alignment>
  using local = basic_local<void *, std::size_t, Bytes, Alignment>;
}