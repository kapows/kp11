#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // int_least8_t, int_least16_t, int_least32_t, int_least64_t, intmax_t, INT_LEAST8_MAX, INT_LEAST16_MAX, INT_LEAST32_MAX, INT_LEAST64_MAX, INTMAX_MAX
#include <type_traits> // conditional_t

namespace kp11
{
  /// Spots stored as an implicit linked list inside of an array of signed integers where each node
  /// denotes it's own number of spots and whether it is vacant or occupied. Vacancies will be
  /// merged on a `reset` if they are adjacent to each other.
  ///
  /// @tparam N Total number of spots.
  template<std::size_t N>
  class list
  {
    static_assert(N <= INTMAX_MAX);

  public: // typedefs
    /// Size type is the smallest signed type possible to reduce our array size.
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
    /// @returns Total number of spots (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }

  public: // modifiers
    /// Forward iterates through the implicit linked list to find an `n` sized vacant node and mark
    /// it as occupied. If a node has more vacant spots than required a new node is created at the
    /// end of the required `n` spots with the remaining vacant spots.
    /// * Complexity `O(n)`.
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns (success) Index of the start of the `n` spots marked occupied.
    /// @returns (failure) `size()`.
    ///
    /// @pre `n > 0`.
    ///
    /// @post (success) Spots from the `(return value)` to `(return value) + n - 1` will not
    /// returned again from any subsequent call to `set` unless `reset` has been called on those
    /// parameters.
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
    /// We consider the node at `index` of size `n`. The adjacent nodes are both checked to
    /// see if they are also vacant. If either are then the vacant nodes are merged into a single
    /// vacant node. Then node is then marked as vacant.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @post `index` to `index + n - 1` may be returned by a call to `set` with appropriate
    /// parameters.
    void reset(size_type index, size_type n) noexcept
    {
      // Need to bounds check 0 because we're using signed types.
      assert(0 <= index && index <= size());
      if (index == size())
      {
        return;
      }
      assert(0 <= n);
      assert(0 <= index + n && index + n <= size());
      assert(sizes[index] == -n && sizes[index + (n - 1)] == -n);
      // Join with previous if it's vacant.
      if (auto const previous = index - 1; index > 0 && sizes[previous] > 0)
      {
        n += sizes[previous];
        index = index - sizes[previous];
      }
      // Join with next if it's vacant.
      if (auto const next = index + n; next < size() && sizes[next] > 0)
      {
        n += sizes[next];
      }
      mark_vacant(index, n);
    }

  private: // helpers
    /// Marks the start and end of the node with the size and occupied sign.
    void mark_occupied(size_type index, size_type n) noexcept
    {
      assert(n > 0);
      sizes[index] = sizes[index + (n - 1)] = -n;
    }
    /// Marks the start and end of the node with the size and vacant sign.
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