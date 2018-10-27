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
    static constexpr size_type max_size() noexcept
    {
      return N;
    }
    /// Forward iterate through the bitset and count consecutive bits.
    /// * Complexity `O(n)`
    ///
    /// @returns The largest number of consecutive unallocated indexes.
    size_type max_alloc() const noexcept
    {
      size_type max = 0;
      size_type count = 0;
      for (std::size_t i = 0; i < N; ++i)
      {
        if (bits[i])
        {
          if (max < count)
          {
            max = count;
          }
          count = 0;
        }
        else
        {
          ++count;
        }
      }
      if (max < count)
      {
        max = count;
      }
      return max;
    }

  public: // modifiers
    /// Forward iterate through the bitset to find `n` adjacent unallocated indexes to allocate. The
    /// algorithm used is more efficient for `n==1`.
    /// * Complexity `O(n)`
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns Index of the start of the `n` indexes allocated.
    ///
    /// @pre `n > 0`
    /// @pre `n <= max_alloc()`
    ///
    /// @post [`(return value)`, `(return value) + n`) will not returned again from any subsequent
    /// call to `allocate` unless it has been `deallocate`d.
    /// @post `count() == (previous) count() + n`.
    size_type allocate(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= max_alloc());
      return n == 1 ? allocate_one() : allocate_many(n);
    }
    /// Forward iterate through the bitset from `index` to `index + n` and deallocate them.
    /// * Complexity `O(n)`
    ///
    /// @param index Return value of a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @post [`index`, `index + n`) may be returned by a call to `allocate`.
    /// @post `count() == (previous) count() - n`
    void deallocate(size_type index, size_type n) noexcept
    {
      assert(n <= max_size());
      assert(index < max_size());
      assert(index + n <= max_size());
      for (auto first = index, last = index + n; first < last; ++first)
      {
        bits.reset(first);
      }
    }

  private: // helper functions
    /// Allocating one is a much simpler algorithm because we don't have to count adjacent bits.
    size_type allocate_one() noexcept
    {
      size_type first = 0;
      for (; bits[first]; ++first)
      {
      }
      bits.set(first);
      return first;
    }
    size_type allocate_many(size_type n) noexcept
    {
      assert(n > 1);
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
      for (auto count = n; count; --count)
      {
        bits.set(--first);
      }
      return first;
    }

  private: // variables
    /// `true` if allocated, `false` if not allocated, this is to be consistent with `bitset::set`.
    std::bitset<N> bits;
  };
}