#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <utility> // exchange

namespace kp11
{
  class stack
  {
  public: // typedefs
    using size_type = std::size_t;

  public: // constructors
    explicit stack(size_type n) noexcept : first(0), length(n)
    {
    }

  public: // capacity
    size_type size() const noexcept
    {
      return length;
    }
    static constexpr size_type max_size() noexcept
    {
      return std::numeric_limits<size_type>::max();
    }

  public: // modifiers
    size_type set(size_type n) noexcept
    {
      if (length - first >= n)
      {
        return std::exchange(first, first + n);
      }
      return size();
    }
    void reset(size_type index, size_type n) noexcept
    {
      if (index + n == first)
      {
        first = index;
      }
    }
    void clear() noexcept
    {
      first = 0;
    }

  private: // variables
    size_type first;
    size_type length;
  };
}