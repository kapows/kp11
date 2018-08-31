#pragma once

#include <cstddef> // size_t

namespace kp11
{
  template<std::size_t N>
  class bitset
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
      return 0;
    }

  public: // modifiers
    /**
     * @copydoc Marker::set
     */
    size_type set(size_type n) noexcept
    {
      return 0;
    }
    /**
     * @copydoc Marker::reset
     */
    void reset(size_type index, size_type n) noexcept
    {
    }
  };
}