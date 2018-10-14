#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <utility> // exchange

namespace kp11
{
  /// @brief LIFO marker. Only supports `reset` of the most recent successful `set` call.
  ///
  /// Spots are stored as a single number. This number is increased by the number of spots that
  /// need to be occupied by a call to `set`. It is only decreased if `reset` is called by the most
  /// recent call to `set`.
  ///
  /// @tparam N Total number of spots.
  template<std::size_t N>
  class stack
  {
  public: // typedefs
    /// Size type.
    using size_type = std::size_t;

  public: // capacity
    /// @returns Number of occupied spots.
    size_type size() const noexcept
    {
      return first;
    }
    /// @returns Total number of spots (`N`).
    static constexpr size_type max_size() noexcept
    {
      return N;
    }
    /// The biggest is always `size()` for this structure.
    /// * Complexity `O(1)`
    ///
    /// @returns The largest number of consecutive vacant spots.
    size_type biggest() const noexcept
    {
      return max_size() - size();
    }

  public: // modifiers
    /// Increases our index by `n` and returns the previous index.
    /// * Complexity `O(1)`
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns Index of the start of the `n` spots marked occupied.
    ///
    /// @pre `n > 0`.
    /// @pre `n <= biggests()`
    ///
    /// @post Spots from the `(return value)` to `(return value) + n - 1` will not
    /// returned again from any subsequent call to `set` unless `reset` has been called on those
    /// parameters and this is the most recent call to `set`.
    /// @post `size() == (previous) size() + n`
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= biggest());
      return std::exchange(first, first + n);
    }
    /// The `index + n` is checked to see whether it is the most recent `set` call. If it is then
    /// our number becomes `index` and thus our first vacant index will start at `index`. If it is
    /// not then it is a no-op.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @post (success) `index` to `index + n - 1` can be returned by a call to `set`.
    /// @post (success) `size() == (previous) size() - n`
    void reset(size_type index, size_type n) noexcept
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