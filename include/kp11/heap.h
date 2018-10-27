#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits

namespace kp11
{
  /// Call `new` on `allocate` and `delete` on `deallocate`.
  class heap
  {
  public: // typedefs
    /// Pointer type is the same type used by `new`.
    using pointer = void *;
    /// Size type is the same type used by `new`.
    using size_type = std::size_t;

  public: // capacity
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return std::numeric_limits<size_type>::max();
    }

  public: // modifiers
    /// Allocate memory by calling `new`.
    ///
    /// @param size Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a suitable memory block.
    /// @returns (failure) `nullptr`
    ///
    /// @post (success) `(return value)` will not be returned again until it has been `deallocated`.
    pointer allocate(size_type size, size_type alignment) noexcept;
    /// Deallocate memory pointed to by `ptr` by calling `delete`.
    ///
    /// @param ptr Pointer return by a call to `allocate`.
    /// @param size Corresponding parameter used in `allocate`.
    /// @param alignment Corresponding parameter used in `allocate`.
    void deallocate(pointer ptr, size_type size, size_type alignment) noexcept;
  };
}