#pragma once

#include <bitset> // bitset
#include <cassert> // assert
#include <cstddef> // size_t

namespace kp11
{
  /// @brief Natural order marker. Iterates through individual bits in a bitset.
  ///
  /// Spots are stored in a bitset, where each bit corresponds to a single spot.
  ///
  /// @tparam N Total number of spots
  template<std::size_t N>
  class bitset
  {
  public: // typedefs
    /// Size type.
    using size_type = std::size_t;

  public: // capacity
    /// @returns Total number of spots (`N`).
    static constexpr size_type max_size() noexcept
    {
      return N;
    }

  public: // modifiers
    /// Forward iterates through the bitset to find `n` adjacent vacant spots and marks them as
    /// occupied. The algorithm used is more efficient for `n==1`.
    /// * Complexity `O(n)`
    ///
    /// @param n Number of spots to mark as occupied.
    ///
    /// @returns (success) Index of the start of the `n` spots marked occupied.
    /// @returns (failure) `max_size()`.
    ///
    /// @pre `n > 0`
    ///
    /// @post (success) Spots from the `(return value)` to `(return value) + n - 1` will not
    /// returned again from any subsequent call to `set` unless `reset` has been called on those
    /// parameters.
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      return n == 1 ? set_one() : set_many(n);
    }
    /// Forward iterates through the bitset from `index` to `index + n` and marks them as vacant.
    /// * Complexity `O(n)`
    ///
    /// @param index Starting index of the spots to mark as vacant.
    /// @param n Number of spots to mark as vacant.
    ///
    /// @pre `index <= max_size()`.
    /// @pre `index + n <= max_size()`.
    ///
    /// @post `index` to `index + n - 1` may be returned by a call to `set` with appropriate
    /// parameters.
    void reset(size_type index, size_type n) noexcept
    {
      // max_size() can be returned by `set` so we'll have to deal with it.
      assert(index <= max_size());
      if (index == max_size())
      {
        return;
      }
      assert(index + n <= max_size());
      for (auto first = index, last = index + n; first < last; ++first)
      {
        bits.reset(first);
      }
    }

  private: // helper functions
    /// Setting one is a much simpler algorithm because we don't have to count adjacent bits.
    size_type set_one() noexcept
    {
      for (size_type first = 0, last = max_size(); first < last; ++first)
      {
        if (!bits[first])
        {
          bits.set(first);
          return first;
        }
      }
      return max_size();
    }
    /// Works for all `n` but inefficient if only setting one.
    size_type set_many(size_type n) noexcept
    {
      for (size_type first = 0, last = max_size(), count = 0; first < last; ++first)
      {
        if (bits[first])
        {
          count = 0;
        }
        else if (++count == n)
        {
          // increment here so we can decrement first without going passed the end
          ++first;
          for (; count; --count)
          {
            bits.set(--first);
          }
          return first;
        }
      }
      return max_size();
    }

  private: // variables
    /// `true` if occupied, `false` if vacant, this is to be consistent with `bitset::set`.
    std::bitset<N> bits;
  };
}