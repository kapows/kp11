#pragma once

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <utility> // exchange

namespace kp11
{
  /**
   * @brief LIFO based marking (size limited) with random order deallocations. The most spots that
   * can be marked by a call to `set` is 1. The most spots that can be vacated by a call to `reset`
   * is 1.
   *
   * @tparam N number of spots
   */
  template<std::size_t N>
  class pool
  {
  public: // typedefs
    /**
     * @brief size type
     */
    using size_type = std::size_t;

  public: // constructors
    /**
     * @brief Construct a new pool object
     */
    pool() noexcept
    {
      for (std::size_t i = 0; i < N; ++i)
      {
        next[i] = i + 1;
      }
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
     * @pre `n == 1`
     *
     * @par Complexity
     * `O(1)`
     */
    size_type set(size_type n) noexcept
    {
      assert(n == 1);
      if (head != size())
      {
        return std::exchange(head, next[head]);
      }
      return size();
    }
    /**
     * @copydoc Marker::reset
     *
     * @pre `n == 1`
     *
     * @par Complexity
     * `O(1)`
     */
    void reset(size_type index, size_type n) noexcept
    {
      assert(n == 1);
      next[index] = head;
      head = index;
    }

  private: // variables
    /**
     * @brief first free index or N
     */
    size_type head = 0;
    /**
     * @brief each index keeps the index of the next index that it points to
     */
    std::array<size_type, N> next;
  };
}