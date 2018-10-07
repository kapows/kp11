#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, uint_least16_t, uint_least32_t, uint_least64_t, uintmax_t, UINT_LEAST8_MAX, UINT_LEAST16_MAX, UINT_LEAST32_MAX, UINT_LEAST64_MAX, UINTMAX_MAX
#include <type_traits> // conditional_t
#include <utility> // exchange


namespace kp11
{
  /// @brief Fixed size LIFO marker. Only supports `set` and `reset` with `n == 1`.
  ///
  /// Spots are stored as a singly linked list inside of an array with each element being a node.
  /// The node points to the next node by using an index. `set` and `reset` calls are each limited
  /// to 1 spot.
  ///
  /// @tparam N Total number of spots.
  template<std::size_t N>
  class pool
  {
    static_assert(N <= UINTMAX_MAX);

  public: // typedefs
    /// Size type is the smallest type possible to reduce our array size.
    using size_type = std::conditional_t<N <= UINT_LEAST8_MAX,
      uint_least8_t,
      std::conditional_t<N <= UINT_LEAST16_MAX,
        uint_least16_t,
        std::conditional_t<N <= UINT_LEAST32_MAX,
          uint_least32_t,
          std::conditional_t<N <= UINT_LEAST64_MAX, uint_least64_t, uintmax_t>>>>;

  public: // constructors
    pool() noexcept
    {
      for (size_type i = 0, last = size(); i < last; ++i)
      {
        next[i] = i + 1;
      }
    }

  public: // capacity
    /// @returns Total number of spots (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }

  public: // modifiers
    /// Checks to see if the head of the linked list is a valid index and marks it as occupied. The
    /// next node becomes the head of the linked list.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns (success) Index of the spot marked occupied.
    /// @returns (failure) `size()`.
    ///
    /// @pre `n == 1`.
    ///
    /// @post (success) `(return value)` will not returned again from any subsequent call to `set`
    /// unless `reset` has been called on it.
    size_type set(size_type n) noexcept
    {
      assert(n == 1);
      if (head != size())
      {
        return std::exchange(head, next[head]);
      }
      return size();
    }
    /// The node at `index` becomes the new head node and the head node is pointed at the previous
    /// head node.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @pre `n == 1`.
    ///
    /// @post `index` may be returned by a call to `set`.
    void reset(size_type index, size_type n) noexcept
    {
      assert(n == 1);
      assert(index <= size());
      if (index == size())
      {
        return;
      }
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