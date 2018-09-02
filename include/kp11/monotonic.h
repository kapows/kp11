#pragma once

#include "traits.h" // is_resource_v

namespace kp11
{
  /**
   * @brief Turn a Resource's `deallocate` into a no-op
   *
   * @tparam Resource type that meets the `Resource` concept
   */
  template<typename Resource>
  class monotonic : public Resource
  {
    static_assert(is_resource_v<Resource>, "monotonic requires Resource to be a Resource");

  public: // typedefs
    using typename Resource::pointer;
    using typename Resource::size_type;

  public: // constructors
    using Resource::Resource;

  public: // modifiers
    /**
     * @copydoc Resource::deallocate
     *
     * @par Complexity
     * `O(0)`
     *
     * @note No-op
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }
  };
}