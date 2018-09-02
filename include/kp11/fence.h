#pragma once

#include "traits.h" // is_strategy_v

#include <cstddef> // size_t

namespace kp11
{
  template<typename Strategy>
  class fence
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  public: // constructors
    fence(pointer, size_type, size_type) noexcept
    {
    }

  public: // modifiers
    pointer allocate(size_type, size_type) noexcept
    {
      return nullptr;
    }
    bool deallocate(pointer, size_type, size_type) noexcept
    {
      return false;
    }

  public: // ownership
    bool owns(pointer, size_type, size_type) noexcept
    {
      return false;
    }
  };
}