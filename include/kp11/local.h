#pragma once

#include "traits.h" // is_strategy_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits

namespace kp11
{
  /// @brief Allocate from a buffer of memory inside itself.
  ///
  /// Only one address will ever be allocated and that is to the start of the buffer. A `bool` is
  /// used to track the whether the buffer has been allocated or not.
  ///
  /// @tparam Pointer Pointer type.
  /// @tparam Size Size type.
  /// @tparam Bytes Size in bytes of the buffer.
  /// @tparam Alignment Alignment in bytes of the buffer.
  template<typename Pointer, typename SizeType, std::size_t Bytes, std::size_t Alignment>
  class basic_local
  {
  public: // typedefs
    /// Pointer type.
    using pointer = Pointer;
    /// Size type.
    using size_type = SizeType;

  private: // typedefs
    /// Byte pointer for arithmetic purposes.
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;
    using byte_pointer_traits = std::pointer_traits<byte_pointer>;

  public: // modifiers
    /// If our memory has not already been allocated and we can fulfil the size request then a
    /// pointer to the beginning of our buffer is allocated.
    /// * Complexity `O(1)`
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of our buffer.
    /// @returns (failure) `nullptr`
    ///
    /// @pre `Alignment % alignment == 0`
    ///
    /// @post (success) (return value) will not be returned again until it has been `deallocated`.
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
    /// If `ptr` points to the beginning of our buffer then we can allocate our buffer again.
    /// * Complexity `O(1)`
    ///
    /// @param ptr Pointer to the beginning of a memory block.
    /// @param bytes Size in bytes of the memory block.
    /// @param alignment Alignment in bytes of the memory block.
    ///
    /// @returns (success) `true`
    /// @returns (failure) `false`
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
    /// Checks whether or not `ptr` points in to memory owned by us.
    ///
    /// @param ptr Pointer to memory.
    ///
    /// @returns (success) Pointer to the beginning of our buffer.
    /// @returns (failure) `nullptr`
    pointer operator[](pointer ptr) noexcept
    {
      if (auto const buf = buffer_ptr();
          std::less_equal<pointer>()(static_cast<pointer>(buf), ptr) &&
          std::less<pointer>()(ptr, static_cast<pointer>(buf + Bytes)))
      {
        return static_cast<pointer>(buf);
      }
      return nullptr;
    }

  private: // helpers
    /// @returns `byte_pointer` created from our inner buffer.
    byte_pointer buffer_ptr() noexcept
    {
      return byte_pointer_traits::pointer_to(buffer[0]);
    }

  private: // variables
    /// Flag whether or not we have allocated our buffer.
    bool allocated = false;
    alignas(Alignment) std::byte buffer[Bytes];
  };

  /// Typedef of basic_local with `void *` as the `pointer` and `std::size_t` as the `size_type`.
  ///
  /// @tparam Bytes Size in bytes of the buffer.
  /// @tparam Alignment Alignment in bytes of the buffer.
  template<std::size_t Bytes, std::size_t Alignment>
  using local = basic_local<void *, std::size_t, Bytes, Alignment>;
}