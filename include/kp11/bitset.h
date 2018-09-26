#pragma once

#include <bitset> // bitset
#include <cstddef> // size_t

namespace kp11
{
  /**
   * @brief Forward iteration based marking using a bitset with random order resets.
   * Vacancies will be summed until the correct number of vacancies is found.
   * Meets the requirements of `Marker`.
   *
   * @tparam N number of spots
   */
  template<std::size_t N>
  class bitset
  {
  public: // typedefs
    /**
     * @brief size type
     */
    using size_type = std::size_t;

  public: // constructors
    /**
     * @brief Construct a new bitset object
     */
    bitset() noexcept
    {
      bits.reset();
    }

  public: // capacity
    /**
     * @copydoc Marker::size
     */
    static constexpr size_type size() noexcept
    {
      return N;
    }

  public: // modifiers
    /**
     * @copydoc Marker::set
     *
     * @par Complexity
     * `O(n)`
     */
    size_type set(size_type n) noexcept
    {
      if (n > size())
      {
        return size();
      }
      auto const last = size() - (n - 1);
      for (size_type i = 0; i < last;)
      {
        auto const count = count_consecutive_vacant_bits(i, i + n);
        // mark and return if found
        if (count == n)
        {
          set(i, n);
          return i;
        }
        // skip ahead if not found (count refers to an occupied spot so we can skip that too thus
        // the + 1)
        i += count + 1;
      }
      return size();
    }
    /**
     * @copydoc Marker::reset
     *
     * @par Complexity
     * `O(n)`
     */
    void reset(size_type index, size_type n) noexcept
    {
      auto const last = index + n;
      for (auto i = index; i < last; ++i)
      {
        bits.reset(i);
      }
    }

  private: // helper functions
    /**
     * @brief Count the number of consecutive vacant bits from `first` until an occupied bit is
     * encountered or until `last` (not included in count).
     *
     * @param first beginning index
     * @param last ending index (not counted)
     * @return count of consecutive vacant bits starting from `first`
     */
    size_type count_consecutive_vacant_bits(size_type first, size_type last) const noexcept
    {
      size_type count = 0;
      for (; first < last; ++first)
      {
        // true is occupied
        if (bits[first] == true)
        {
          break;
        }
        ++count;
      }
      return count;
    }
    /**
     * @brief private helper function as the opposite of `reset`
     *
     * @param index index of first bit to set
     * @param n number of bits to set
     */
    void set(size_type index, size_type n) noexcept
    {
      auto const last = index + n;
      for (auto i = index; i < last; ++i)
      {
        bits.set(i);
      }
    }

  private: // variables
    /**
     * @brief bitset true if occupied, false if vacant, this is to be consistent with bitset `set`
     * and `reset`.
     */
    std::bitset<N> bits;
  };
}