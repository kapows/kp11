#pragma once

#include "traits.h" // is_resource_v

namespace kp11
{
  /**
   * @brief If allocation from `Primary` is unsuccessful then allocates from `Fallback`
   *
   * @tparam Primary type that meets the `Resource` concept
   * @tparam Fallback type that meets the `Resource` concept
   */
  template<typename Primary, typename Fallback>
  class fallback
  {
    static_assert(is_resource_v<Primary>, "fallback requires Primary to be a Resource");
    static_assert(is_resource_v<Fallback>, "fallback requires Fallback to be a Resource");

  public: // typedefs
    /**
     * @brief pointer type
     */
    using pointer = typename Primary::pointer;
    /**
     * @brief size type
     */
    using size_type = typename Primary::size_type;

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (auto ptr = primary.allocate(bytes, alignment))
      {
        return ptr;
      }
      return fallback.allocate(bytes, alignment);
    }
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (!primary.deallocate(ptr, bytes, alignment))
      {
        fallback.deallocate(ptr, bytes, alignment);
      }
    }

  public: // accessors
    /**
     * @brief Get the primary object
     */
    Primary & get_primary() noexcept
    {
      return primary;
    }
    /**
     * @brief Get the primary object
     */
    Primary const & get_primary() const noexcept
    {
      return primary;
    }
    /**
     * @brief Get the fallback object
     */
    Fallback & get_fallback() noexcept
    {
      return fallback;
    }
    /**
     * @brief Get the fallback object
     */
    Fallback const & get_fallback() const noexcept
    {
      return fallback;
    }

  private: // variables
    Primary primary;
    Fallback fallback;
  };
}