#pragma once

#include "traits.h" // is_resource_v

namespace kp11
{
  /**
   * @brief If allocation from `Primary` is unsuccessful then allocates from `Fallback`
   *
   * @tparam Primary type that meets the `Resource` concept
   * @tparam Fallback type that meets the `Resource` concept
   * @pre Primary must either return a convertible bool value on dellocate or return a convertible
   * bool value from operator[](pointer ptr) in order to determine ownership.
   */
  template<typename Primary, typename Fallback>
  class fallback : private Primary, private Fallback
  {
    static_assert(is_resource_v<Primary>);
    static_assert(is_resource_v<Fallback>);

  public: // typedefs
    using typename Primary::pointer;
    using typename Primary::size_type;

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (auto ptr = Primary::allocate(bytes, alignment))
      {
        return ptr;
      }
      return Fallback::allocate(bytes, alignment);
    }
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      // it may be trivial for a type to return success or failure in it's deallocate function, if
      // if is then it should do so
      if constexpr (std::is_convertible_v<bool,
                      decltype(std::declval<Primary>().deallocate(std::declval<pointer>(),
                        std::declval<size_type>(),
                        std::declval<size_type>()))>)
      {
        if (!Primary::deallocate(ptr, bytes, alignment))
        {
          Fallback::deallocate(ptr, bytes, alignment);
        }
      }
      // if it is not trivial then perhaps we can still determine ownership through operator[]
      // it is an error to use types that do not expose operator[] or return a convertible bool on
      // deallocate as Primary
      else
      {
        if (Primary::operator[](ptr))
        {
          Primary::deallocate(ptr, bytes, alignment);
        }
        else
        {
          Fallback::deallocate(ptr, bytes, alignment);
        }
      }
    }

  public: // accessors
    /**
     * @brief Get the primary object
     */
    Primary & get_primary() noexcept
    {
      return *this;
    }
    /**
     * @brief Get the primary object
     */
    Primary const & get_primary() const noexcept
    {
      return *this;
    }
    /**
     * @brief Get the fallback object
     */
    Fallback & get_fallback() noexcept
    {
      return *this;
    }
    /**
     * @brief Get the fallback object
     */
    Fallback const & get_fallback() const noexcept
    {
      return *this;
    }
  };
}