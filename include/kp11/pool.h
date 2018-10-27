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
      for (size_type i = 0, last = max_size(); i < last; ++i)
      {
        next[i] = i + 1;
      }
    }

  public: // capacity
    /// @returns Number of allocated indexes.
    size_type size() const noexcept
    {
      return num_occupied;
    }
    /// @returns Total number of indexes (`N`).
    static constexpr size_type max_size() noexcept
    {
      return static_cast<size_type>(N);
    }
    /// @returns `1` if there are unallocated indexes otherwise `0`.
    size_type max_alloc() const noexcept
    {
      return head != max_size() ? static_cast<size_type>(1) : static_cast<size_type>(0);
    }

  public: // modifiers
    /// The next node becomes the head of the linked list. Returns the index of the previous head
    /// node.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns Index of the start of the `n` indexes to allocate.
    ///
    /// @pre `n == 1`
    /// @pre `n <= max_alloc()`
    ///
    /// @post `(return value)` will not returned again from any subsequent call to `allocate`
    /// unless `deallocate` has been called on it.
    /// @post `size() == (previous) size() + n`
    size_type allocate(size_type n) noexcept
    {
      assert(n == 1);
      assert(n <= max_alloc());
      ++num_occupied;
      return std::exchange(head, next[head]);
    }
    /// The node at `index` becomes the new head node and the head node is pointed at the previous
    /// head node.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @pre `n == 1`
    ///
    /// @post `index` may be returned by a call to `allocate`.
    /// @post `size() == (previous) size() - n`
    void deallocate(size_type index, size_type n) noexcept
    {
      assert(n == 1);
      assert(index < max_size());
      --num_occupied;
      next[index] = head;
      head = index;
    }

  private: // variables
    size_type num_occupied = 0;
    /// First free index or `N`.
    size_type head = 0;
    /// Holds the index of the next free index.
    std::array<size_type, N> next;
  };
}