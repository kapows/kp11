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
    size_type size() noexcept
    {
      return max_size() - first;
    }
    /// @returns Total number of spots (`N`).
    static constexpr size_type max_size() noexcept
    {
      return N;
    }

  public: // modifiers
    /// Compares our number with `max_size()` to see if we have `n` vacant spots and increases our
    /// number by `n`.
    /// * Complexity `O(1)`.
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns (success) Index of the start of the `n` spots marked occupied.
    /// @returns (failure) `max_size()`.
    ///
    /// @pre `n > 0`.
    ///
    /// @post (success) Spots from the `(return value)` to `(return value) + n - 1` will not
    /// returned again from any subsequent call to `set` unless `reset` has been called on those
    /// parameters and this is the most recent call to `set`.
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      if (max_size() - first >= n)
      {
        return std::exchange(first, first + n);
      }
      return max_size();
    }
    /// The `index + n` is checked to see whether it is the most recent `set` call. If it is then
    /// our number becomes `index` and thus our first vacant index will start at `index`. If it is
    /// not then it is a no-op.
    /// * Complexity `O(1)`
    ///
    /// @param index Returned by a call to `set`.
    /// @param n Corresponding parameter used in `set`.
    ///
    /// @post `index` to `index + n - 1` may be returned by a call to `set` with appropriate
    /// parameters if these parameters are from the most recent call to `set`.
    void reset(size_type index, size_type n) noexcept
    {
      assert(index <= max_size());
      if (index == max_size())
      {
        return;
      }
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