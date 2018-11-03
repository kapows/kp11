#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, uint_least16_t, uint_least32_t, uint_least64_t, uintmax_t, UINT_LEAST8_MAX, UINT_LEAST16_MAX, UINT_LEAST32_MAX, UINT_LEAST64_MAX, UINTMAX_MAX
#include <type_traits> // conditional_t
#include <utility> // exchange

namespace kp11
{
  /// @brief Fixed size LIFO marker. Only supports `allocate` and `deallocate` with `n == 1`.
  ///
  /// Indexes are stored as a singly linked list inside of an array with each element being a node.
  /// The node points to the next node by using an index. `allocate` and `deallocate` calls are each
  /// limited to 1 index.
  ///
  /// @tparam N Total number of indexes.
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
    /// @returns Number of allocated indexes.
    size_type count() const noexcept
    {
      return num_occupied;
    }
    /// @returns Total number of indexes (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }
    /// @returns The maximum allocation size supported. This is always `1`.
    static constexpr size_type max_size() noexcept
    {
      return static_cast<size_type>(1);
    }

  public: // modifiers
    /// The next node becomes the head of the linked list. Returns the index of the previous head
    /// node.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns (success) Index of the start of the `n` indexes to allocate.
    /// @returns (failure) `size()`
    ///
    /// @pre `n == 1`
    /// @pre `n <= max_size()`
    ///
    /// @post `(return value)` will not returned again from any subsequent call to `allocate`
    /// unless `deallocate` has been called on it.
    /// @post `count() == (previous) count() + n`
    size_type allocate(size_type n) noexcept
    {
      assert(n == 1);
      assert(n <= max_size());
      if (head != size())
      {
        ++num_occupied;
        return std::exchange(head, next[head]);
      }
      return size();
    }
    /// The node at `i` becomes the new head node and the head node is pointed at the previous
    /// head node.
    /// * Complexity `O(1)`
    ///
    /// @param i Returned by a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @pre `n == 1`
    ///
    /// @post `i` may be returned by a call to `allocate`.
    /// @post `count() == (previous) count() - n`
    void deallocate(size_type i, size_type n) noexcept
    {
      assert(n == 1);
      assert(i < size());
      --num_occupied;
      next[i] = head;
      head = i;
    }

  private: // variables
    size_type num_occupied = 0;
    /// First free index or `N`.
    size_type head = 0;
    /// Holds the index of the next free index.
    std::array<size_type, N> next;
  };
}