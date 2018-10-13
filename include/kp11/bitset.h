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
    /// @returns Number of vacant spots.
    size_type size() const noexcept
    {
      return num_vacant;
    }
    /// @returns Total number of spots (`N`).
    static constexpr size_type max_size() noexcept
    {
      return N;
    }
    /// Iterates through the bitset and counts consecutive bits.
    /// * Complexity `O(n)`
    ///
    /// @returns The largest number of consecutive vacant spots.
    size_type biggest() const noexcept
    {
      size_type biggest = 0;
      size_type count = 0;
      for (std::size_t i = 0; i < N; ++i)
      {
        if (bits[i])
        {
          if (biggest < count)
          {
            biggest = count;
          }
          count = 0;
        }
        else
        {
          ++count;
        }
      }
      if (biggest < count)
      {
        biggest = count;
      }
      return biggest;
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
    /// @pre `n <= biggest()`
    ///
    /// @post (success) Spots from the `(return value)` to `(return value) + n - 1` will not
    /// returned again from any subsequent call to `set` unless `reset` has been called on those
    /// parameters.
    /// @post (success) `size() == (previous) size() - n`.
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= biggest());
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
    /// @post (success) `index` to `index + n - 1` may be returned by a call to `set` with
    /// appropriate parameters.
    /// @post (success) `size() == (previous) size() + n`.
    void reset(size_type index, size_type n) noexcept
    {
      assert(n <= max_size());
      assert(index < max_size());
      assert(index + n <= max_size());
      num_vacant += n;
      for (auto first = index, last = index + n; first < last; ++first)
      {
        bits.reset(first);
      }
    }

  private: // helper functions
    /// Setting one is a much simpler algorithm because we don't have to count adjacent bits.
    size_type set_one() noexcept
    {
      size_type first = 0;
      for (; bits[first]; ++first)
      {
      }
      --num_vacant;
      bits.set(first);
      return first;
    }
    /// Works for all `n` but inefficient if only setting one.
    size_type set_many(size_type n) noexcept
    {
      size_type first = 0;
      for (size_type count = 0; count < n; ++first)
      {
        if (bits[first])
        {
          count = 0;
        }
        else
        {
          ++count;
        }
      }
      num_vacant -= n;
      for (auto count = n; count; --count)
      {
        bits.set(--first);
      }
      return first;
    }

  private: // variables
    /// Number of vacant spots.
    size_type num_vacant = N;
    /// `true` if occupied, `false` if vacant, this is to be consistent with `bitset::set`.
    std::bitset<N> bits;
  };
}