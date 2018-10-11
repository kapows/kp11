#pragma once

#include "detail/static_vector.h" // static_vector

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, UINT_LEAST8_MAX

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
  /// @brief Unordered marker. Iterates through a free list.
  ///
  /// Free list is stored as a `size` and `index` inside of an array. A cache of the free list index
  /// is kept for each index to make merges `O(1)`. Occupied spots will have `size()` as their value
  /// in the cache. Vacancies will be merged on a `reset` if they are adjacent to each other. The
  /// biggest size available is cached, so a request that can't be met is `O(1)`. A request of 1 is
  /// `O(1)`, all other request are `O(n)`.
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
      if constexpr (size() > 0)
      {
        auto & node = free_list.emplace_back(size(), 0);
        auto node_index = static_cast<size_type>(free_list.size() - 1);
        set_cache(node.index, node.size, node_index);
      }
    }

  public: // capacity
    /// @returns Total number of spots (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }

  public: // modifiers
    /// Forward iterate through the free list to find an `n` sized vacant node. If the size of the
    /// selected node is bigger than `n` then the node will be shrunk in the free list. If the size
    /// is the same as `n` then the node will be removed. If the chosen free list node was the
    /// biggest then a new biggest node will be cached by searching the entire free list.
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
      if (free_list.empty() || free_list[biggest_node_index].size < n)
      {
        return size();
      }
      size_type node_index = 0;
      size_type replacement_biggest_node_index = 0;
      // We're guaranteed to find a suitable size since n is at maximum, the biggest size we have.
      // Don't need to guard againt going off the end here, we'll just find the element.
      for (; free_list[node_index].size < n; ++node_index)
      {
        // The replacement will always be 1 behind the node_index, this is because node_index will
        // be modified. If it is then we can continue the search from node_index after modification
        // has taken place.
        if (free_list[replacement_biggest_node_index].size < free_list[node_index].size)
        {
          replacement_biggest_node_index = node_index;
        }
      }
      auto replace_biggest_node_index = node_index == biggest_node_index;

      auto & node = free_list[node_index];
      // index is the return value, we'll have to copy it because node will be modified.
      auto const index = node.index;
      set_cache(index, n, size());
      if (node.size == n)
      {
        remove_node(node_index);
      }
      else
      {
        node.size -= n;
        node.index += n;
        set_cache(node.index, node.size, node_index);
      }
      if (replace_biggest_node_index)
      {
        // Search the rest of the free list for the biggest node.
        for (auto last = free_list.size(); node_index != last; ++node_index)
        {
          if (free_list[replacement_biggest_node_index].size < free_list[node_index].size)
          {
            replacement_biggest_node_index = node_index;
          }
        }
        biggest_node_index = replacement_biggest_node_index;
      }
      return index;
    }
    /// Checks to see if the node either sits at a boundary or has an adjacent node on either
    /// side. If it has adjacent nodes then they are checked to see whether or not they are vacant.
    /// If there are two vacant adjacent nodes then we will merge them into one node whilst removing
    /// the other.
    /// If there is one vacant adjacent node then we will merge with that node.
    /// If there are no vacant adjacent nodes then we will add a new node to our free list.
    /// If merging or adding a new node results in a node bigger than our biggest node, then it
    /// becomes the new biggest node.
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

      size_type node_index;
      auto const previous_cache_index = index - 1;
      auto const previous_is_vacant = index > 0 && cache[previous_cache_index] != size();
      auto const next_cache_index = index + n;
      auto const next_is_vacant = index + n < size() && cache[next_cache_index] != size();
      if (previous_is_vacant && next_is_vacant)
      {
        // There will be 2 active nodes, so we'll remove the next node.
        node_index = cache[previous_cache_index];
        auto const next_node_index = cache[next_cache_index];
        free_list[node_index].size += n + free_list[next_node_index].size;
        remove_node(next_node_index);
      }
      else if (previous_is_vacant)
      {
        // Combine all sizes into the previous node.
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
      if (free_list[biggest_node_index].size < node.size)
      {
        biggest_node_index = node_index;
      }
      set_cache(node.index, node.size, node_index);
    }

  private: // helpers
    /// Cache setting helper because we'll have to set both the start and end.
    void set_cache(size_type index, size_type size, size_type node_index) noexcept
    {
      assert(index < this->size());
      assert(index + size - 1 < this->size());
      assert(size > 0);
      cache[index + (size - 1)] = cache[index] = node_index;
    }
    /// Node removal helper because we'll have to update the cache of the node that replaces the
    /// removed node. Note the removed node does not get cache updating. Order is not guaranteed. If
    /// the biggest node was moved from the back the the biggest node index will change to match.
    void remove_node(size_type index) noexcept
    {
      assert(index < free_list.size());
      auto & node = free_list[index];
      node = free_list.back();
      if (index != free_list.size() - 1)
      {
        set_cache(node.index, node.size, index);
      }
      free_list.pop_back();
      // biggest node was moved
      if (biggest_node_index == free_list.size())
      {
        biggest_node_index = index;
      }
    }

  private: // variables
    /// Biggest sized node
    size_type biggest_node_index = 0;
    /// Free list stores it's own size and index.
    /// `N / 2 + N % 2` because that is the maximum number of free list nodes we will ever have
    /// (this will happen when we have an alternating vacant, occupied, vacant, occupied pattern).
    ///
    /// Example: Assume size() == 11, then
    /// [(9, 2), (2, 3)]
    kp11::detail::static_vector<node, N / 2 + N % 2> free_list;
    /// Cache stores an index into the free list for each run. The index is stored at the beginning
    /// and the end of the run. If the run is size 1 then the index is only stored in one element.
    /// If the run is not in the free list (it's been occupied) then `size()` is used as its index.
    /// We'll need the cache to do merges in `O(1)` time.
    ///
    /// Example: Assume size() == 11, then
    /// [11, 11, 1, X, 1, 11, X, X, 11, 0, 0, 11]
    /// X is just a placeholder here for garbage indexes.
    std::array<size_type, N> cache;
  };
}