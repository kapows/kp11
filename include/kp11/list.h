#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // int_least8_t, int_least16_t, int_least32_t, int_least64_t, intmax_t, INT_LEAST8_MAX, INT_LEAST16_MAX, INT_LEAST32_MAX, INT_LEAST64_MAX, INTMAX_MAX
#include <type_traits> // conditional_t

namespace kp11
{
  /// Forward iteration based marking using an implicit linked list with random order resets.
  /// Vacancies from `reset`s will be merged if they are adjacent to each other.
  /// * `N` is the number of spots
  template<std::size_t N>
  class list
  {
    // we'll need signed integers
    static_assert(N <= INTMAX_MAX);

  public: // typedefs
    /// Pick the smallest type possible to reduce our array size
    using size_type = std::conditional_t<N <= INT_LEAST8_MAX,
      int_least8_t,
      std::conditional_t<N <= INT_LEAST16_MAX,
        int_least16_t,
        std::conditional_t<N <= INT_LEAST32_MAX,
          int_least32_t,
          std::conditional_t<N <= INT_LEAST64_MAX, int_least64_t, intmax_t>>>>;

  public: // constructors
    list() noexcept
    {
      if (size() > 0)
      {
        mark_vacant(0, size());
      }
    }

  public: // capacity
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }

  public: // modifiers
    /// * Complexity `O(n)`
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      for (size_type i = 0, last = size(); i < last; i += (sizes[i] < 0 ? -sizes[i] : sizes[i]))
      {
        if (sizes[i] >= n)
        {
          // Marking changes sizes[i] so leftover needs to be calculated before hand.
          auto const leftover = sizes[i] - n;
          // Mark occupied first so that we're always moving forward.
          mark_occupied(i, n);
          if (leftover)
          {
            mark_vacant(i + n, leftover);
          }
          return i;
        }
      }
      return size();
    }
    /// * Complexity `O(1)`
    void reset(size_type index, size_type n) noexcept
    {
      assert(0 <= index && index < size());
      assert(0 <= n && n <= size());
      assert(0 <= index + n && index + n <= size());
      assert(sizes[index] == -n && sizes[index + (n - 1)] == -n);
      // join with previous if it's vacant
      if (auto const previous = index - 1; index > 0 && sizes[previous] > 0)
      {
        n += sizes[previous];
        index = index - sizes[previous];
      }
      // join with next if it's vacant
      if (auto const next = index + n; next < size() && sizes[next] > 0)
      {
        n += sizes[next];
      }
      mark_vacant(index, n);
    }

  private: // helpers
    /// Requires `n > 0`.
    void mark_occupied(size_type index, size_type n) noexcept
    {
      assert(n > 0);
      sizes[index] = sizes[index + (n - 1)] = -n;
    }
    /// Requires `n > 0`.
    void mark_vacant(size_type index, size_type n) noexcept
    {
      assert(n > 0);
      sizes[index] = sizes[index + (n - 1)] = n;
    }

  private: // variables
    /// Implicit linked list that stores it's own size (number of spots until next). The size is
    /// stored both in the beginning and the end of the spots that it occupies. If the size is 1
    /// then it only occupies 1 spot. Positive for vacant, negative
    /// if occupied.
    ///
    /// Example: [-2, -2, 3, 0, 3, -4, 0, 0, -4, 2, 2, -1]
    /// 0 is not necessarily 0 but a placeholder for garbage characters.
    /// Here 0 is 2 wide and occupied, 2 is 3 wide and vacant, 5 is 4 wide and occupied, 9 is 2 wide
    /// and vacant, 10 is 1 wide and occupied.
    std::array<size_type, N> sizes;
  };
}