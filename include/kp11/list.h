#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // int_least8_t, int_least16_t, int_least32_t, int_least64_t, intmax_t, INT_LEAST8_MAX, INT_LEAST16_MAX, INT_LEAST32_MAX, INT_LEAST64_MAX, INTMAX_MAX
#include <type_traits> // conditional_t

namespace kp11
{
  /// @brief Unordered marker. Iterates through a free list.
  ///
  /// All nodes are stored inside of an array with their size and free list index, size is negative
  /// if the node is occupied. The free list index is stored so make allow indexing on merges. Free
  /// list nodes are stored inside of an array with their size and node index. The biggest node in
  /// the free list is always at the back, otherwise there is no ordering. Vacancies will be merged
  /// on a `reset` if they are adjacent to each other.
  ///
  /// @tparam N Total number of spots.
  template<std::size_t N>
  class list
  {
    static_assert(N <= INTMAX_MAX);

  public: // typedefs
    /// Size type is the smallest unsigned type possible to reduce our array size.

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
    /// If `n` is bigger than the last node in our free list just return `size()`. Else we are
    /// definately able to fulfil the request so forward iterates through the free list to find an
    /// `n` sized vacant node index and remove it from the free list. If the selected node is the
    /// back of the free list then the next biggest node will take it's place. Mark [`index`, `index
    /// + n`) as occupied in the all node array. If `n` is smaller than the node's size mark [`index
    /// + n`, `index + old_size`) as vacant and add it to the free list.
    /// * Complexity `n==1` is `O(1)`, otherwise `O(n)`.
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns (success) Index of the start of the `n` spots marked occupied.
    /// @returns (failure) `size()`.
    ///
    /// @pre `n > 0`.
    ///
    /// @post (success) Spots [`(return value)`, `(return value) + n`) will not returned again from
    /// any subsequent call to `set` unless `reset` has been called on those parameters.
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
    /// see if they are also vacant. If either are then the vacant nodes are removed from the free
    /// list and are merged into a single vacant node. Then node is then marked as vacant and added
    /// to the free list.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @post [`index`, `index + n`) may be returned by a call to `set` with appropriate parameters.
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
    /// Nodes that stores it's own size and index into `free_list`. The size is stored both in the
    /// beginning and the end of node. If the size is 1 then it only occupies 1 spot. Vacant spots
    /// will have free list `index < size()`, occupied nodes will have `index == size()`.
    ///
    /// Example: Assume size() == 11, then
    /// [(11, 2), (11, 2), (1, 3), 0, (1, 3), (11, 4), 0, 0, (11, 4), (0, 2), (0, 2), (11, 1)]
    /// 0 is not necessarily 0 but a placeholder for garbage characters.
    /// Notice that the node at index 2 has free list index 1, as it is the largest node.
    std::array<size_type, N> sizes;
    /// Free list stores it's own size and index into `sizes`. The biggest node at the back.
    /// `N / 2 + N % 2` because that is the maximum number of free list nodes we will ever have
    /// (this will happen when we have an alternating vacant, occupied, vacant, occupied pattern).
    ///
    /// Example:
    /// [(9, 2), (2, 3)]
  };
}