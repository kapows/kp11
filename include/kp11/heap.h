#pragma once

#include <cstddef> // size_t

namespace kp11
{
  /// Calls `new` on `allocate` and `delete` on `deallocate`.
  /// Meets the `Resource` concept.
  class heap
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  public: // modifiers
    /// Calls `new`.
    pointer allocate(size_type bytes, size_type alignment) noexcept;
    /// Calls `delete`.
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
  };
}