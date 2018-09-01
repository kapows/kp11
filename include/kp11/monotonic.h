#pragma once

#include "traits.h" // is_strategy_v

namespace kp11
{
  /**
   * @brief Turn a Strategy's `deallocate` into a no-op
   *
   * @tparam Strategy type that meets the `Strategy` concept
   */
  template<typename Strategy>
  class monotonic : public Strategy
  {
    static_assert(is_strategy_v<Strategy>, "monotonic requires Strategy to be a Strategy");

  public: // typedefs
    using typename Strategy::pointer;
    using typename Strategy::size_type;

  public: // constructors
    using Strategy::Strategy;

  public: // modifiers
    /**
     * @copydoc Strategy::deallocate
     *
     * @par Complexity
     * `O(0)`
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }
  };
}