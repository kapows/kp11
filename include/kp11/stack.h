#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <utility> // exchange

namespace kp11
{
  /// LIFO based marking with reverse ordered resets.
  /// * `N` is the number of spots
  template<std::size_t N>
  class stack
  {
  public: // typedefs
    using size_type = std::size_t;

  public: // capacity
    static constexpr size_type size() noexcept
    {
      return N;
    }

  public: // modifiers
    /// * Complexity `O(1)`
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      if (size() - first >= n)
      {
        return std::exchange(first, first + n);
      }
      return size();
    }
    /// Only the most recent `set` can be recovered.
    /// * Complexity `O(1)`
    void reset(size_type index, size_type n) noexcept
    {
      assert(index <= size());
      assert(n <= size());
      if (index == size())
      {
        return;
      }
      assert(index + n <= size());
      assert(index < first);
      if (index + n == first)
      {
        first = index;
      }
    }

  private: // variables
    /// Current index.
    size_type first = 0;
  };
}