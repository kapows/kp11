#pragma once

#include "detail/static_vector.h" // static_vector

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, UINT_LEAST8_MAX
#include <utility> // swap

namespace kp11
{
  namespace list_detail
  {
    /// @private
    /// Free list node type.
    template<typename SizeType>
    struct node
    {
    public: // typedefs
      using size_type = SizeType;

    public: // constructors
      node(size_type size, size_type index) noexcept : size(size), index(index)
      {
      }

    public: // variables
      size_type size;
      size_type index;
    };
  }
  /// @brief Unordered best fit marker. Iterates through a free list.
  ///
  /// Free list is stored as a `size` and `index` inside of an array. A cache of the free list index
  /// is kept for each spot. Occupied spots will have `max_size()` as their value in the cache.
  /// Vacancies will be merged on a `reset` if they are adjacent to each other. Merges are `O(1)`.
  /// * Complexity `O(n)`
  ///
  /// @tparam N Total number of spots.
  template<std::size_t N>
  class list
  {
    static_assert(N <= UINT_LEAST8_MAX);

  public: // typedefs
    /// Size type is the smallest unsigned type possible to reduce our array size.
    using size_type = uint_least8_t;

  private: // typedefs
    using node = list_detail::node<size_type>;

  public: // constructors
    list() noexcept
    {
      if constexpr (max_size() > 0)
      {
        auto & node = free_list.emplace_back(max_size(), 0);
        auto node_index = static_cast<size_type>(free_list.size() - 1);
        set_cache(node.index, node.size, node_index);
      }
    }

  public: // capacity
    /// @returns Number of occupied spots.
    size_type size() const noexcept
    {
      return num_occupied;
    }
    /// @returns Total number of spots (`N`).
    static constexpr size_type max_size() noexcept
    {
      return static_cast<size_type>(N);
    }
    /// Forward iterates through the free list to find the largest number of consecutive spots.
    /// * Complexity `O(n)`
    ///
    /// @returns The largest number of consecutive vacant spots.
    size_type biggest() const noexcept
    {
      size_type largest = 0;
      for (auto && node : free_list)
      {
        if (largest < node.size)
        {
          largest = node.size;
        }
      }
      return largest;
    }

  public: // modifiers
    /// Forward iterate through the free list to find the best fit node for `n`. If the size of the
    /// selected node is bigger than `n` then the node will be shrunk in the free list. If the size
    /// is the same as `n` then the node will be removed.
    /// * Complexity `O(n)`.
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns Index of the start of the `n` spots marked occupied.
    ///
    /// @pre `n > 0`.
    /// @pre `n <= biggest()`
    ///
    /// @post Spots [`(return value)`, `(return value) + n`) will not returned again from
    /// any subsequent call to `set` unless `reset` has been called on those parameters.
    /// @post `size() == (previous) size() + n`.
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= biggest());
      size_type node_index = 0;
      // Find first fit
      for (; free_list[node_index].size < n; ++node_index)
      {
      }
      // Find best fit
      for (size_type i = node_index + 1, last = static_cast<size_type>(free_list.size()); i != last;
           ++i)
      {
        if (n <= free_list[i].size && free_list[i].size < free_list[node_index].size)
        {
          node_index = i;
        }
      }
      auto & node = free_list[node_index];
      node.size -= n;
      auto const index = node.index + node.size;
      set_cache(index, n, max_size());
      if (node.size > 0)
      {
        set_cache(node.index, node.size, node_index);
      }
      else
      {
        remove_node(node_index);
      }
      num_occupied += n;
      return index;
    }
    /// If the node has adjacent nodes then they are checked to see whether or not they are vacant.
    /// If there are two vacant adjacent nodes then merge them into one node whilst removing the
    /// other. If there is one vacant adjacent node then merge with that node. If there are no
    /// vacant adjacent nodes then add a new node to the free list.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @post [`index`, `index + n`) may be returned by a call to `set` with appropriate parameters.
    /// @post `size() == (previous) size() - n`.
    void reset(size_type index, size_type n) noexcept
    {
      assert(index < max_size());
      assert(n > 0);
      assert(index + n <= max_size());
      num_occupied -= n;
      size_type node_index;
      auto const previous_cache_index = index - 1;
      auto const previous_is_vacant = index > 0 && cache[previous_cache_index] != max_size();
      auto const next_cache_index = index + n;
      auto const next_is_vacant = index + n < max_size() && cache[next_cache_index] != max_size();
      if (previous_is_vacant && next_is_vacant)
      {
        // There will be 2 active nodes. Keep previous and remove next.
        node_index = cache[previous_cache_index];
        auto const next_node_index = cache[next_cache_index];
        free_list[node_index].size += n + free_list[next_node_index].size;
        free_list[next_node_index].size = 0;
        remove_node(next_node_index);
      }
      else if (previous_is_vacant)
      {
        node_index = cache[previous_cache_index];
        free_list[node_index].size += n;
      }
      else if (next_is_vacant)
      {
        node_index = cache[next_cache_index];
        free_list[node_index].size += n;
        free_list[node_index].index = index;
      }
      else
      {
        free_list.emplace_back(n, index);
        node_index = static_cast<size_type>(free_list.size() - 1);
      }
      auto & node = free_list[node_index];
      set_cache(node.index, node.size, node_index);
    }

  private: // helpers
    /// Cache setting helper because both the start and end must be set.
    void set_cache(size_type index, size_type size, size_type node_index) noexcept
    {
      assert(index < max_size());
      assert(size > 0);
      assert(index + size <= max_size());
      cache[index + (size - 1)] = cache[index] = node_index;
    }
    /// Node removal helper because the cache needs to be kept in sync.
    /// Note the removed node does not get cache updating. Order is not guaranteed.
    void remove_node(size_type node_index) noexcept
    {
      assert(node_index < free_list.size());
      assert(free_list[node_index].size == 0);
      if (auto const back_node_index = static_cast<size_type>(free_list.size() - 1);
          node_index != back_node_index)
      {
        free_list[node_index] = free_list[back_node_index];
        set_cache(free_list[node_index].index, free_list[node_index].size, node_index);
      }
      free_list.pop_back();
    }

  private: // variables
    /// Number of occupied spots.
    size_type num_occupied = 0;
    /// Free list stores it's own size and index.
    /// `N / 2 + N % 2` because that is the maximum number of free list nodes we will ever have
    /// (this will happen when we have an alternating vacant, occupied, vacant, occupied pattern).
    ///
    /// Example: Assume size() == 11, then
    /// [(2, 9), (3, 2)]
    kp11::detail::static_vector<node, N / 2 + N % 2> free_list;
    /// Cache stores an index into the free list for each run. The index is stored at the beginning
    /// and the end of the run. If the run is size 1 then the index is only stored in one element.
    /// If the run is not in the free list (it's been occupied) then `size()` is used as its index.
    /// Cache enables merges in to be `O(1)`.
    ///
    /// Example: Assume size() == 11, then
    /// [11, 11, 1, X, 1, 11, X, X, 11, 0, 0, 11]
    /// X is just a placeholder here for garbage indexes.
    std::array<size_type, N> cache;
  };
}