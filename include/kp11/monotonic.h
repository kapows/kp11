#pragma once

#include <cstddef> // size_t

namespace kp11
{
  template<typename Strategy>
  class monotonic
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  public: // constructors
    monotonic(pointer ptr, size_type bytes, size_type alignment)
    {
    }

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }
  };
}