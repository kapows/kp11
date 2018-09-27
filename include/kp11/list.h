#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <utility> // exchange

namespace kp11
{
  /// Forward iteration based marking using an implicit linked list with random order resets.
  /// Vacancies from resets will be merged during a `set` call if they are adjacent to each other
  /// and are currently being searched.
  /// * `N` number of spots
  template<std::size_t N>
  class list
  {
    static_assert(N <= std::numeric_limits<std::ptrdiff_t>::max(),
      "list must have N <= std::numeric_limits<std::ptrdiff_t>::max()");

  public: // typedefs
    using size_type = std::size_t;

  private: // typedefs
    using difference_type = std::ptrdiff_t;

  public: // constructors
    list() noexcept
    {
      if (N > 0)
      {
        next[0] = N;
      }
      for (size_type i = 1; i < N; ++i)
      {
        next[i] = 0;
      }
    }

  public: // capacity
    static constexpr size_type size() noexcept
    {
      return N;
    }

  public: // modifiers
    /// * Complexity `O(n)`
    size_type set(size_type n) noexcept
    {
      size_type i = 0;
      while (i != size())
      {
        // skip if already occupied
        if (next[i] < 0)
        {
          i -= next[i];
          continue;
        }
        // vacant, we'll merge all other vacancies from reset calls
        for (auto j = i + next[i]; j != size() && next[j] > 0; j = i + next[i])
        {
          next[i] += std::exchange(next[j], 0);
        }
        // skip if still not enough
        if (next[i] < n)
        {
          i += next[i];
          continue;
        }
        // split if there are more vacant spots than necessary
        if (next[i] != n)
        {
          next[i + n] = next[i] - n;
        }
        // mark as occupied
        next[i] = -n;
        return i;
      }
      return size();
    }
    /// * Complexity `O(1)`
    void reset(size_type index, size_type n) noexcept
    {
      assert(next[index] == -static_cast<difference_type>(n));
      next[index] = n;
    }

  private: // variables
    /// Implicit linked list that stores it's own size (number of spots until next).
    /// Positive for vacant, negative if occupied, and 0 if the spot is part of another.
    std::array<difference_type, N> next;
  };
}