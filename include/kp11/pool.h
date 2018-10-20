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
      for (size_type i = 0, last = max_size(); i < last; ++i)
      {
        next[i] = i + 1;
      }
    }

  public: // capacity
    /// Iterate through the linked list to find the number of occupied spots.
    /// * Complexity `O(n)`
    ///
    /// @returns Number of occupied spots.
    size_type size() const noexcept
    {
      size_type num_occupied = max_size();
      for (auto x = head; x != max_size(); x = next[x])
      {
        --num_occupied;
      }
      return num_occupied;
    }
    /// @returns Total number of spots (`N`).
    static constexpr size_type max_size() noexcept
    {
      return static_cast<size_type>(N);
    }
    /// @returns `1` if there are vacant spots otherwise `0`.
    size_type biggest() const noexcept
    {
      return head != max_size() ? static_cast<size_type>(1) : static_cast<size_type>(0);
    }

  public: // modifiers
    /// The next node becomes the head of the linked list. Returns the previous head node.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns Index of the spot marked occupied.
    ///
    /// @pre `n == 1`
    /// @pre `n <= biggest()`
    ///
    /// @post `(return value)` will not returned again from any subsequent call to `set`
    /// unless `reset` has been called on it.
    /// @post `size() == (previous) size() + n`
    size_type set(size_type n) noexcept
    {
      assert(n == 1);
      assert(n <= biggest());
      return std::exchange(head, next[head]);
    }
    /// The node at `index` becomes the new head node and the head node is pointed at the previous
    /// head node.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @pre `n == 1`
    ///
    /// @post `index` may be returned by a call to `set`.
    /// @post `size() == (previous) size() - n`
    void reset(size_type index, size_type n) noexcept
    {
      assert(n == 1);
      assert(index < max_size());
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