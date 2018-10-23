#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less, less_equal
#include <memory> // pointer_traits

namespace kp11
{
  /// @brief Allocate from a buffer.
  ///
  /// Only one address will ever be allocated and that is to the start of the buffer. A `bool` is
  /// used to track the whether the buffer has been allocated or not.
  ///
  /// @tparam Pointer Pointer type.
  /// @tparam Size Size type.
  template<typename Pointer, typename SizeType>
  class basic_buffer
  {
  public: // typedefs
    /// Pointer type.
    using pointer = Pointer;
    /// Size type.
    using size_type = SizeType;

  private: // typedefs
    /// Byte pointer for arithmetic purposes.
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;

  public: // constructors
    /// Default is defined because other constructor is defined.
    basic_buffer() = default;
    /// @param ptr Pointer to a memory block.
    /// @param size Size in bytes of memory block.
    /// @param alignment Alignment in bytes of memory block.
    basic_buffer(pointer ptr, size_type size, size_type alignment) noexcept :
        ptr(static_cast<byte_pointer>(ptr)), size(size), alignment(alignment)
    {
    }

  public: // modifiers
    /// If our buffer has not already been allocated and we can fulfil the size request then a
    /// pointer to our buffer is allocated.
    /// * Complexity `O(1)`
    ///
    /// @param size Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the our buffer.
    /// @returns (failure) `nullptr`
    ///
    /// @pre `alignment (from ctor) % alignment == 0`
    ///
    /// @post (success) `(return value)` will not be returned again until it has been `deallocated`.
    pointer allocate(size_type size, [[maybe_unused]] size_type alignment) noexcept
    {
      assert(this->alignment % alignment == 0);
      if (!allocated && size <= this->size)
      {
        allocated = true;
        return static_cast<pointer>(ptr);
      }
      return nullptr;
    }
    /// If `ptr` points to the our buffer then we can allocate our buffer again.
    /// * Complexity `O(1)`
    ///
    /// @param ptr Pointer to the beginning of a memory block.
    /// @param size Size in bytes of the memory block.
    /// @param alignment Alignment in bytes of the memory block.
    ///
    /// @returns (success) `true`
    /// @returns (failure) `false`
    bool deallocate(
      pointer ptr, [[maybe_unused]] size_type size, [[maybe_unused]] size_type alignment) noexcept
    {
      if (static_cast<byte_pointer>(ptr) == this->ptr)
      {
        allocated = false;
        return true;
      }
      return false;
    }

  public: // observers
    /// Checks whether or not `ptr` points in to our buffer.
    ///
    /// @param ptr Pointer to memory.
    ///
    /// @returns (success) Pointer to our buffer.
    /// @returns (failure) `nullptr`
    pointer operator[](pointer ptr) noexcept
    {
      if (std::less_equal<pointer>()(static_cast<pointer>(this->ptr), ptr) &&
          std::less<pointer>()(ptr, static_cast<pointer>(this->ptr + size)))
      {
        return static_cast<pointer>(this->ptr);
      }
      return nullptr;
    }

  private: // variables
    /// Flag whether or not we have allocated our buffer.
    bool allocated = false;
    byte_pointer ptr;
    size_type size;
    size_type alignment;
  };

  /// Typedef of basic_buffer with `void *` as the `pointer` and `std::size_t` as the `size_type`.
  using buffer = basic_buffer<void *, std::size_t>;
}