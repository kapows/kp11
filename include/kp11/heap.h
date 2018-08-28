#pragma once

#include <cstddef> // size_t

namespace kp11
{
  class heap
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept;
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
  };
}