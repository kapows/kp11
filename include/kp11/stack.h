#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <utility> // exchange

namespace kp11
{
  /// @brief LIFO marker. Only recovers `deallocate`d indexes if they are adjacent to the the
  /// current unallocated index.
  ///
  /// Indexes are stored as a single number. This number is increased by the number of indexes that
  /// need to be allocated. It is only decreased if when deallocating the most recently allocated
  /// indexes.
  ///
  /// @tparam N Total number of indexes.
  template<std::size_t N>
  class stack
  {
  public: // typedefs
    /// Size type.
    using size_type = std::size_t;

  public: // capacity
    /// @returns Number of allocated indexes.
    size_type size() const noexcept
    {
      return first;
    }
    /// @returns Total number of indexes (`N`).
    static constexpr size_type max_size() noexcept
    {
      return N;
    }
    /// The biggest is always `max_size() - size()` for this structure.
    /// * Complexity `O(1)`
    ///
    /// @returns The largest number of consecutive unallocated indexes.
    size_type biggest() const noexcept
    {
      return max_size() - size();
    }

  public: // modifiers
    /// Increases our index by `n` and returns the previous index.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns Index of the start of the `n` indexes allocated.
    ///
    /// @pre `n > 0`.
    /// @pre `n <= biggests()`
    ///
    /// @post [`(return value)`, `(return value) + n`) will not returned again from any subsequent
    /// call to `allocate` unless properly deallocated.
    /// @post `size() == (previous) size() + n`
    size_type allocate(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= biggest());
      return std::exchange(first, first + n);
    }
    /// The `index + n` is checked to see whether it is adjacent to our number. If it is then our
    /// number becomes `index` and thus our first unallocated index will start at `index`. If it is
    /// not then it is a no-op and the indexes are not recovered.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @post (success) [`index`, `index + n`) can be returned by a call to `allocate`.
    /// @post (success) `size() == (previous) size() - n`
    void deallocate(size_type index, size_type n) noexcept
    {
      assert(index <= max_size());
      assert(index + n <= max_size());
      assert(index < first);
      if (index + n == first)
      {
        first = index;
      }
    }

  private: // variables
    /// Current index.
    size_type first = 0;
  };
}