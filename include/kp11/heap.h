#pragma once

#include <cstddef> // size_t

namespace kp11
{
  /// Calls `new` on `allocate` and `delete` on `deallocate`.
  class heap
  {
  public: // typedefs
    /// Pointer type is the same type used by `new`.
    using pointer = void *;
    /// Size type is the same type used by `new`.
    using size_type = std::size_t;

  public: // modifiers
    /// Allocate memory by calling `new`.
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    /// @returns (failure) `nullptr`.
    pointer allocate(size_type bytes, size_type alignment) noexcept;
    /// Deallocate memory pointed to by `ptr` by calling `delete`.
    ///
    /// @param ptr Pointer return by a call to `allocate`.
    /// @param bytes Corresponding parameter used in `allocate`.
    /// @param alignment Corresponding parameter used in `allocate`.
    ///
    /// @pre A deallocate call with `ptr` can only be called once after a return from `allocate`.
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
  };
}