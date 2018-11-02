#pragma once

#include "detail/static_vector.h" // static_vector

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, UINT_LEAST8_MAX
#include <utility> // swap, exchange

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
  /// Free list is stored as a `size` and `index` inside of an array.
  /// An array is used to cache an index's index in the free list.
  /// The cache uses the beginning and end of an index range to store it's index into the free list.
  /// Allocated index ranges will have `size()` as their value in the cache.
  /// Unallocated indexes will be merged on a `deallocate` if they are adjacent to each other.
  /// Merges are `O(1)`.
  /// * Complexity `O(n)`
  ///
  /// @tparam N Total number of indexes.
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
        push_back(0, size());
      }
    }

  public: // capacity
    /// Forward iterates through the free list to count the number of allocated indexes.
    /// * Complexity `O(n)`
    ///
    /// @returns Number of allocated indexes.
    size_type count() const noexcept
    {
      auto num_allocated = size();
      for (auto && node : free_list)
      {
        num_allocated -= node.size;
      }
      return num_allocated;
    }
    /// @returns Total number of indexes (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return size();
    }

  public: // modifiers
    /// Forward iterate through the free list to find the best fit node for `n`. If the size of the
    /// selected node is bigger than `n` then the node will be shrunk in the free list. If the size
    /// is the same as `n` then the node will be removed.
    /// * Complexity `O(n)`.
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns (success) Index of the start of the `n` indexes allocated.
    /// @returns (failure) `size()`
    ///
    /// @pre `n > 0`.
    /// @pre `n <= max_size()`
    ///
    /// @post [`(return value)`, `(return value) + n`) will not returned again from
    /// any subsequent call to `allocate` unless deallocated.
    /// @post `count() == (previous) count() + n`.
    size_type allocate(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= max_size());
      if (auto const i = find_best_fit(n); i != size())
      {
        return take_front(i, n);
      }
      return size();
    }
    /// If the node has adjacent nodes then they are checked to see whether or not they are
    /// unallocated. If there are two unallocated adjacent nodes then merge them into one node
    /// whilst removing the other. If there is one unallocated adjacent node then merge with that
    /// node. If there are no unallocated adjacent nodes then add a new node to the free list.
    /// * Complexity `O(1)`
    ///
    /// @param i Returned by a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @post [`i`, `i + n`) may be returned by a call to `allocate`.
    /// @post `count() == (previous) count() - n`.
    void deallocate(size_type i, size_type n) noexcept
    {
      assert(i < size());
      assert(n > 0);
      assert(i + n <= size());
      auto const previous_is_vacant = i > 0 && cache[i - 1] != size();
      auto const next_is_vacant = i + n < size() && cache[i + n] != size();
      if (previous_is_vacant)
      {
        auto const j = free_list[cache[i - 1]].index;
        add_back(j, n);
        if (next_is_vacant)
        {
          auto const next = free_list[cache[i + n]];
          take_front(next.index, next.size);
          add_back(j, next.size);
        }
      }
      else if (next_is_vacant)
      {
        add_front(i + n, n);
      }
      else
      {
        push_back(i, n);
      }
    }

  private: // helpers
    /// Cache set helper because both the start and end must be set.
    void set_cache(size_type i, size_type n, size_type node_index) noexcept
    {
      assert(i < size());
      assert(n > 0);
      assert(i + n <= size());
      cache[i + (n - 1)] = cache[i] = node_index;
    }
    /// Node removal helper because the cache needs to be kept in sync.
    /// Move and pop with back.
    /// Note the removed node does not get cache updating.
    /// Invalidates all free list indexes.
    void remove_node(size_type node_index) noexcept
    {
      assert(node_index < free_list.size());
      assert(free_list[node_index].size == 0);
      auto && node = free_list[node_index];
      node = free_list.back();
      if (node.size)
      {
        set_cache(node.index, node.size, node_index);
      }
      free_list.pop_back();
    }
    /// Forward iterate through the free list to find the best fit indexes for `n`.
    ///
    /// @pre `n > 0`
    /// @pre `n <= max_size()`
    ///
    /// @returns (success) Index of `n` unallocated indexes.
    /// @returns (failure) `size()`.
    size_type find_best_fit(size_type n) const noexcept
    {
      assert(n > 0);
      assert(n <= max_size());
      size_type node_index = size();
      for (size_type i = 0, last = static_cast<size_type>(free_list.size()); i != last; ++i)
      {
        if (n <= free_list[i].size &&
            (node_index == size() || free_list[i].size < free_list[node_index].size))
        {
          node_index = i;
          // Exact fit is best fit
          if (free_list[node_index].size == n)
          {
            break;
          }
        }
      }
      if (node_index != size())
      {
        return free_list[node_index].index;
      }
      return size();
    }
    /// Takes `n` indexes out of the front of the free list node belonging to `i` and sets
    /// the cache to size(). If the number of indexes in the free list node not zero, the cache
    /// for the new `i` is updated, otherwise, the node is removed. Invalidates the beginning
    /// index in the cache.
    size_type take_front(size_type i, size_type n) noexcept
    {
      assert(i < this->size());
      assert(n > 0);
      assert(cache[i] != this->size());
      auto node_index = cache[i];
      auto && node = free_list[node_index];
      assert(node.size >= n);
      node.size -= n;
      auto const taken_index = std::exchange(node.index, node.index + n);
      set_cache(taken_index, n, this->size());
      if (node.size)
      {
        set_cache(node.index, node.size, node_index);
      }
      else
      {
        remove_node(node_index);
      }
      return taken_index;
    }
    /// Adds `n` unallocated indexes to the back of the free list node belonging to `index` and
    /// sets the cache. Invalidates the end index in the cache.
    void add_back(size_type i, size_type n) noexcept
    {
      assert(i < size());
      assert(cache[i] != size());
      auto const a = cache[i];
      auto && node = free_list[a];
      node.size += n;
      set_cache(node.index, node.size, a);
    }
    /// Adds `size` unallocated indexes to the front of the free list node belonging to `index` and
    /// sets the cache. Invalidates the beginning index in the cache.
    void add_front(size_type i, size_type n) noexcept
    {
      assert(i < size());
      assert(cache[i] != size());
      auto const a = cache[i];
      auto && node = free_list[a];
      node.size += n;
      node.index -= n;
      set_cache(node.index, node.size, a);
    }
    /// Add a node to the back of the free list and sets the cache for the new node.
    void push_back(size_type i, size_type n) noexcept
    {
      assert(i < this->size());
      assert(n > 0);
      free_list.emplace_back(n, i);
      auto const a = static_cast<size_type>(free_list.size() - 1);
      set_cache(free_list[a].index, free_list[a].size, a);
    }

  private: // variables
    /// Free list stores it's own size and index.
    /// `N / 2 + N % 2` because that is the maximum number of free list nodes we will ever have
    /// (this will happen when we have an alternating unallocated, allocated, unallocated pattern).
    ///
    /// Example: Assume `size() == 11`, then
    /// [(2, 9), (3, 2)]
    kp11::detail::static_vector<node, N / 2 + N % 2> free_list;
    /// Cache stores an index into the free list for each run of unallocated indexes. The index is
    /// stored at the beginning and the end of the run. If the run is size 1 then the index is only
    /// stored in one element. If the run is not in the free list (it's been occupied) then
    /// `size()` is used as its index. Cache enables merges in to be `O(1)`.
    ///
    /// Example: Assume `size() == 11`, b is the beginning index and e is the end, then
    /// [11, 11, 1, X, 1, 11, X, X, 11, 0, 0, 11]
    ///          b     e                b  e
    /// X is just a placeholder here for garbage indexes.
    std::array<size_type, N> cache;
  };
}