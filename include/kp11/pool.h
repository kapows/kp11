#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <utility> // exchange

namespace kp11
{
  /// LIFO based marking (size limited) with random order deallocations.
  /// The most spots that can be marked by a call to `set` is 1.
  /// The most spots that can be vacated by a call to `reset` is 1.
  /// * `N` is the number of spots
  template<std::size_t N>
  class pool
  {
  public: // typedefs
    using size_type = std::size_t;

  public: // constructors
    pool() noexcept
    {
      for (size_type i = 0, last = size(); i < last; ++i)
      {
        next[i] = i + 1;
      }
    }

  public: // capacity
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }

  public: // modifiers
    /// * Precondition `n == 1`
    /// * Complexity `O(1)`
    size_type set(size_type n) noexcept
    {
      assert(n == 1);
      if (head != size())
      {
        return std::exchange(head, next[head]);
      }
      return size();
    }
    /// * Precondition `n == 1`
    /// * Complexity `O(1)`
    void reset(size_type index, size_type n) noexcept
    {
      assert(n == 1);
      next[index] = head;
      head = index;
    }

  private: // variables
    /// First free index or `N`.
    size_type head = 0;
    /// Holds the index of the next free index.
    std::array<size_type, N> next;
  };
}