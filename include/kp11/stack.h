#pragma once

#include <cstddef> // size_t

namespace kp11
{
  class stack
  {
  public: // typedefs
    using size_type = std::size_t;

  public: // constructors
    explicit stack(size_type n) noexcept
    {
    }

  public: // capacity
    size_type size() const noexcept
    {
      return 0;
    }
    static constexpr size_type max_size() noexcept
    {
      return 0;
    }

  public: // modifiers
    size_type set(size_type n) noexcept
    {
      return 0;
    }
    void reset(size_type index, size_type n) noexcept
    {
    }
    void clear() noexcept
    {
    }
  };
}