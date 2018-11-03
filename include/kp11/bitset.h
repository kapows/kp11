#pragma once

#include <bitset> // bitset
#include <cassert> // assert
#include <cstddef> // size_t

namespace kp11
{
  /// @brief Natural order marker. Iterates through individual bits in a bitset.
  ///
  /// Indexes are stored as a bitset, where each bit corresponds to an index.
  ///
  /// @tparam N Total number of indexes
  template<std::size_t N>
  class bitset
  {
  public: // typedefs
    /// Size type.
    using size_type = std::size_t;

  public: // capacity
    /// @returns Number of allocated indexes.
    size_type count() const noexcept
    {
      return bits.count();
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
    /// Forward iterate through the bitset to find `n` adjacent unallocated indexes to allocate. The
    /// algorithm used is more efficient for `n==1`.
    /// * Complexity `O(n)`
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns (success) Index of the start of the `n` indexes allocated.
    /// @returns (failure) `size()`
    ///
    /// @pre `n > 0`
    /// @pre `n <= max_size()`
    ///
    /// @post [`(return value)`, `(return value) + n`) will not returned again from any subsequent
    /// call to `allocate` unless it has been `deallocate`d.
    /// @post `count() == (previous) count() + n`.
    size_type allocate(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= max_size());
      return n == 1 ? allocate_one() : allocate_many(n);
    }
    /// Forward iterate through the bitset from `i` to `i + n` and deallocate them.
    /// * Complexity `O(n)`
    ///
    /// @param i Return value of a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @post [`i`, `i + n`) may be returned by a call to `allocate`.
    /// @post `count() == (previous) count() - n`
    void deallocate(size_type i, size_type n) noexcept
    {
      assert(n <= size());
      assert(i < size());
      assert(i + n <= size());
      for (auto first = i, last = i + n; first < last; ++first)
      {
        bits.reset(first);
      }
    }

  private: // helper functions
    /// Allocating one is a much simpler algorithm because we don't have to count adjacent bits.
    size_type allocate_one() noexcept
    {
      size_type first = 0;
      for (; first != size() && bits[first]; ++first)
      {
      }
      if (first != size())
      {
        bits.set(first);
        return first;
      }
      return size();
    }
    size_type allocate_many(size_type n) noexcept
    {
      assert(n > 1);
      size_type first = 0;
      for (size_type count = 0; first != size(); ++first)
      {
        if (bits[first])
        {
          count = 0;
        }
        else if (++count == n)
        {
          // have to increment first before we can decrement the count since we're off by 1
          ++first;
          for (auto count = n; count; --count)
          {
            bits.set(--first);
          }
          return first;
        }
      }
      return size();
    }

  private: // variables
    /// `true` if allocated, `false` if not allocated, this is to be consistent with
    /// `bitset::set`.
    std::bitset<N> bits;
  };
}