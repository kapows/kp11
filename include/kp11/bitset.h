#pragma once

#include <bitset> // bitset
#include <cassert> // assert
#include <cstddef> // size_t

namespace kp11
{
  /// Forward iteration based marking using a bitset with random order resets.
  /// Vacancies will be summed until the correct number of vacancies is found.
  /// * `N` is the number of spots
  template<std::size_t N>
  class bitset
  {
  public: // typedefs
    using size_type = std::size_t;

  public: // capacity
    static constexpr size_type size() noexcept
    {
      return N;
    }

  public: // modifiers
    /// * Complexity `O(n)`
    size_type set(size_type n) noexcept
    {
      assert(n > 0);
      return n == 1 ? set_one() : set_many(n);
    }
    /// * Complexity `O(n)`
    void reset(size_type index, size_type n) noexcept
    {
      assert(index < size());
      assert(n <= size());
      assert(index + n <= size());
      for (auto first = index, last = index + n; first < last; ++first)
      {
        bits.reset(first);
      }
    }

  private: // helper functions
    size_type set_one() noexcept
    {
      for (size_type first = 0, last = size(); first < last; ++first)
      {
        if (!bits[first])
        {
          bits.set(first);
          return first;
        }
      }
      return size();
    }
    size_type set_many(size_type n) noexcept
    {
      for (size_type first = 0, last = size(), count = 0; first < last; ++first)
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
      return size();
    }

  private: // variables
    /// `true` if occupied, `false` if vacant, this is to be consistent with `bitset::set`.
    std::bitset<N> bits;
  };
}