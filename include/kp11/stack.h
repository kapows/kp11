#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <utility> // exchange

namespace kp11
{
  /**
   * @brief LIFO based marking with reverse ordered resets. Any other resets will do nothing.
   */
  class stack
  {
  public: // typedefs
    /**
     * @brief size type
     *
     */
    using size_type = std::size_t;

  public: // constructors
    /**
     * @brief Construct a new stack object
     *
     * @param n number of vacant spots to start with.
     */
    explicit stack(size_type n) noexcept : first(0), length(n)
    {
    }

  public: // capacity
    /**
     * @copydoc Marker::size
     */
    size_type size() const noexcept
    {
      return length;
    }
    /**
     * @return maximum number of vacant spots
     */
    static constexpr size_type max_size() noexcept
    {
      return std::numeric_limits<size_type>::max();
    }

  public: // modifiers
    /**
     * @copydoc Marker::set
     */
    size_type set(size_type n) noexcept
    {
      if (length - first >= n)
      {
        return std::exchange(first, first + n);
      }
      return size();
    }
    /**
     * @copydoc Marker::reset
     *
     * @note Although any index returned by `set` can be used only the `index` and `n` of the most
     *    recent `set` call will vacate occupied spots.
     */
    void reset(size_type index, size_type n) noexcept
    {
      if (index + n == first)
      {
        first = index;
      }
    }
    /**
     * @brief Clear all occupied spots (makes them all vacant).
     */
    void clear() noexcept
    {
      first = 0;
    }

  private: // variables
    size_type first;
    size_type length;
  };
}