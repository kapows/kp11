#pragma once

#include <cstddef> // size_t
#include <utility> // exchange

namespace kp11
{
  /**
   * @brief LIFO based marking with reverse ordered resets. Any other resets will do nothing.
   *
   * @tparam N number of spots
   */
  template<std::size_t N>
  class stack
  {
  public: // typedefs
    /**
     * @brief size type
     */
    using size_type = std::size_t;

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
     */
    size_type set(size_type n) noexcept
    {
      if (N - first >= n)
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

  private: // variables
    size_type first = 0;
  };
}