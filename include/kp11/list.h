#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uint_least8_t, UINT_LEAST8_MAX

namespace kp11
{
  namespace list_detail
  {
    /// @private
    /// Implicit linked list node type.
    template<typename SizeType>
    struct run
    {
    public: // typedefs
      using size_type = SizeType;

    public: // variables
      // Number of indexes available in the run. Same as `size` if available otherwise `0`.
      size_type available;
      // Number of indexes in the run
      size_type size;
    };
  }
  /// @brief FIFO first fit marker. Iterates through a free list.
  ///
  /// Unallocated indexes will be merged on a `deallocate` if they are adjacent to each other.
  /// Merges are `O(1)`.
  /// * Complexity `O(n)`
  ///
  /// @tparam N Total number of indexes.
  template<std::size_t N>
  class list
  {
    static_assert(N <= UINT_LEAST8_MAX);

  public: // typedefs
    /// Size type is the smallest unsigned type possible to reduce our array size. Any bigger and it
    /// might take forever to search.
    using size_type = uint_least8_t;

  private: // typedefs
    using run = list_detail::run<size_type>;

  public: // constructors
    list() noexcept
    {
      if constexpr (size() > 0)
      {
        set_run(0, size(), size());
      }
    }

  public: // capacity
    /// Forward iterates through the list to count the number of allocated indexes.
    /// * Complexity `O(n)`
    ///
    /// @returns Number of allocated indexes.
    size_type count() const noexcept
    {
      size_type n = size();
      for (size_type i = 0; i != size(); i += runs[i].size)
      {
        n -= runs[i].available;
      }
      return n;
    }
    /// @returns Total number of indexes (`N`).
    static constexpr size_type size() noexcept
    {
      return static_cast<size_type>(N);
    }
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return size();
    }

  public: // modifiers
    /// Forward iterate through the runs to find the first unallocated run for `n`. If there are
    /// leftovers the run will be split and they will remain unallocated.
    /// * Complexity `O(n)`.
    ///
    /// @param n Number of indexes to allocate.
    ///
    /// @returns (success) Index of the start of the `n` indexes allocated.
    /// @returns (failure) `size()`
    ///
    /// @pre `n > 0`.
    /// @pre `n <= max_size()`
    ///
    /// @post [`(return value)`, `(return value) + n`) will not returned again from
    /// any subsequent call to `allocate` unless deallocated.
    /// @post `count() == (previous) count() + n`.
    size_type allocate(size_type n) noexcept
    {
      assert(n > 0);
      assert(n <= max_size());
      if (auto const i = find_first_fit(n); i != size())
      {
        auto const m = runs[i].available - n;
        // leftover
        if (m)
        {
          set_run(i, m, m);
        }
        auto const j = i + m;
        set_run(j, n, 0);
        return j;
      }
      return size();
    }
    /// If there are unallocated adjacent runs on either side they are merged.
    /// * Complexity `O(1)`
    ///
    /// @param i Returned by a call to `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    ///
    /// @post [`i`, `i + n`) may be returned by a call to `allocate`.
    /// @post `count() == (previous) count() - n`.
    void deallocate(size_type i, size_type n) noexcept
    {
      assert(i < size());
      assert(n > 0);
      assert(i + n <= size());
      assert(runs[i].available == 0);
      assert(runs[i].size == n);
      assert(runs[i + (n - 1)].available == 0);
      assert(runs[i + (n - 1)].size == n);
      if (auto const prev = i - 1; i > 0 && runs[prev].available)
      {
        i -= runs[prev].size;
        n += runs[prev].size;
      }
      if (auto const next = i + n; next < size() && runs[next].available)
      {
        n += runs[next].size;
      }
      set_run(i, n, n);
    }

  private: // helpers
    /// Exists because both the start and end of the run must be set.
    void set_run(size_type i, size_type n, size_type a) noexcept
    {
      assert(i < size());
      assert(i + n <= size());
      assert(n > 0);
      runs[i] = runs[i + (n - 1)] = {a, n};
    }
    /// Forward iterate through the list to find the first unallocated run for `n`.
    ///
    /// @pre `n > 0`
    /// @pre `n <= max_size()`
    ///
    /// @returns (success) Index of `n` unallocated indexes.
    /// @returns (failure) `size()`.
    size_type find_first_fit(size_type n) const noexcept
    {
      assert(n > 0);
      assert(n <= max_size());
      for (size_type i = 0; i != size(); i += runs[i].size)
      {
        if (n <= runs[i].available)
        {
          return i;
        }
      }
      return size();
    }

  private: // variables
    /// The availability and size is stored at the beginning and the end of each run.
    /// If the run is size 1 then the index is only stored in one element.
    /// Only the beginning and end of each run is valid, all other elements are garbage.
    /// Storing at both the beginning and end of the run enables merges in to be `O(1)`.
    /// To iterate the run, the size must be added to the index.
    ///
    /// Example: Assume `size() == 11`
    /// [(2,2), (2,2), (0,3), X, (0,3), (6,6), X, X, X, X, (6,6)]
    ///  |---free---|  |--allocated--|  |---------free---------|
    /// X is just a placeholder here for garbage.
    std::array<run, N> runs;
  };
}