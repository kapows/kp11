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
    size_type count() const noexcept
    {
      return first;
    }
    /// @returns Total number of indexes (`N`).
    static constexpr size_type size() noexcept
    {
      return N;
    }
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return size();
    }

  public: // modifiers
    /// Increases our index by `n` and returns the previous index.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns (success) Index of the start of the `n` indexes allocated.
    /// @returns (failure) `size()`
    ///
    /// @pre `n > 0`.
    /// @pre `n <= max_size()`
    ///
    /// @post [`(return value)`, `(return value) + n`) will not returned again from any subsequent
    /// call to `allocate` unless properly deallocated.
    /// @post `count() == (previous) count() + n`
    size_type allocate(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= max_size());
      if (size() - first >= n)
      {
        return std::exchange(first, first + n);
      }
      return size();
    }
    /// The `i + n` is checked to see whether it is adjacent to our unallocated index. If it is
    /// then our unallocated index will start at `i`. If not then it is a no-op and the indexes
    /// are not recovered.
    /// * Complexity `O(1)`
    ///
    /// @param i Returned by a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @post (success) [`i`, `i + n`) can be returned by a call to `allocate`.
    void deallocate(size_type i, size_type n) noexcept
    {
      assert(i <= size());
      assert(i + n <= size());
      assert(i < first);
      if (i + n == first)
      {
        first = i;
      }
    }

  private: // variables
    /// Current index.
    size_type first = 0;
  };
}