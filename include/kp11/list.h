#pragma once

#include "detail/static_vector.h" // static_vector

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, uint_least16_t, uint_least32_t, uint_least64_t, uintmax_t, UINT_LEAST8_MAX, UINT_LEAST16_MAX, UINT_LEAST32_MAX, UINT_LEAST64_MAX, UINTMAX_MAX
#include <type_traits> // conditional_t

namespace kp11
{
  /// @brief Unordered marker. Iterates through a free list.
  ///
  /// All nodes are stored inside of an array with their size and free list index, size is negative
  /// if the node is occupied. The free list index is stored so make allow indexing on merges. Free
  /// list nodes are stored inside of an array with their size and node index. The biggest size
  /// available in the free list will be also be stored separately, this is to allow early returning
  /// when we cannot fulfil a request. Vacancies will be merged on a `reset` if they are adjacent to
  /// each other.
  ///
  /// @tparam N Total number of spots.
  template<std::size_t N>
  class list
  {
    static_assert(N <= UINTMAX_MAX);

  public: // typedefs
    /// Size type is the smallest unsigned type possible to reduce our array size.
    using size_type = std::conditional_t<N <= UINT_LEAST8_MAX,
      uint_least8_t,
      std::conditional_t<N <= UINT_LEAST16_MAX,
        uint_least16_t,
        std::conditional_t<N <= UINT_LEAST32_MAX,
          uint_least32_t,
          std::conditional_t<N <= UINT_LEAST64_MAX, uint_least64_t, uintmax_t>>>>;

  public: // typedefs
    /// Internal node type
    struct node
    {
      size_type index;
      size_type size;
    };

  public: // constructors
    list() noexcept
    {
      if (size() > 0)
      {
        free_list.push_back({0, size()});
        mark_vacant(0, size(), 0);
      }
    }

  public: // capacity
    /// @returns Total number of spots (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }

  public: // modifiers
    /// If `n` is bigger than the the biggest size in our free list just return `size()`. Else we
    /// are definately able to fulfil the request so forward iterates through the free list to find
    /// an `n` sized vacant node index. If the size of the selected node is bigger than `n` then the
    /// node will be shrunk in the free list. If the size is the same as `n` then the node will be
    /// removed. [`index`, `index + n`) is marked occupied in the all node array. If there are
    /// leftovers then mark [`index + n`, `index + old_size`) as vacant.
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
      if (free_list.empty())
      {
        return size();
      }
      auto first = free_list.begin();
      auto last = free_list.end();
      for (; first != last; ++first)
      {
        if (n <= first->size)
        {
          break;
        }
      }
      if (first == last)
      {
        return size();
      }
      auto selected = *first;
      if (n < selected.size)
      {
        node leftover = {
          static_cast<size_type>(selected.index + n), static_cast<size_type>(selected.size - n)};
        mark_occupied(selected.index, selected.size);
        mark_vacant(
          leftover.index, leftover.size, static_cast<size_type>(first - free_list.begin()));
      }
      else
      {
        *first = free_list.back();
        mark_vacant(first->index, first->size, static_cast<size_type>(first - free_list.begin()));
        free_list.pop_back();
      }
      return selected.index;
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
      assert(index <= size());
      if (index == size())
      {
        return;
      }
      assert(index + n <= size());
      // Join with previous if it's vacant.
      auto const previous = index - 1;
      auto const next = index + n;
      bool join_with_previous = index > 0 && all[previous].index != size();
      bool join_with_next = next < size() && all[next].index != size();
      if (join_with_previous && join_with_next)
      {
        // We'll have to remove one of them so we'll remove next.
        mark_vacant(previous, all[previous].size + n + all[next].size, all[previous].index);
        free_list[all[next].index] = free_list.back();
        free_list.pop_back();
      }
      else if (join_with_previous)
      {
        mark_vacant(previous, all[previous].size + n, all[previous].index);
      }
      else if (join_with_next)
      {
        mark_vacant(index, n + all[next].size, all[next].index);
        free_list[all[index].index] = {index, all[index].size};
      }
      else
      {
        free_list.push_back({index, n});
        mark_vacant(index, n, static_cast<size_type>(free_list.size() - 1));
      }
    }

  private: // helpers
    /// Marks the start and end of the node with the size and invalid free list index.
    void mark_occupied(size_type index, size_type n) noexcept
    {
      all[index + (n - 1)] = all[index] = {size(), n};
    }
    /// Marks the start and end of the node with the size and free list index.
    void mark_vacant(size_type index, size_type n, size_type free_list_index) noexcept
    {
      all[index + (n - 1)] = all[index] = {free_list_index, n};
      free_list[free_list_index] = {index, n};
    }

  private: // variables
    /// Nodes that stores it's own size and index into `free_list`. The size is stored both in the
    /// beginning and the end of node. If the size is 1 then it only occupies 1 spot. Vacant spots
    /// will have free list `index < size()`, occupied nodes will have `index == size()`.
    ///
    /// Example: Assume size() == 11, then
    /// [(11, 2), (11, 2), (1, 3), 0, (1, 3), (11, 4), 0, 0, (11, 4), (0, 2), (0, 2), (11, 1)]
    /// 0 is not necessarily 0 but a placeholder for garbage characters.
    std::array<node, N> all;
    /// Free list stores it's own size and index into `sizes`.
    /// `N / 2 + N % 2` because that is the maximum number of free list nodes we will ever have
    /// (this will happen when we have an alternating vacant, occupied, vacant, occupied pattern).
    ///
    /// Example:
    /// [(9, 2), (2, 3)]
    kp11::detail::static_vector<node, N / 2 + N % 2> free_list;
    // size_type biggest = N;
  };
}