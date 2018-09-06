#pragma once

namespace kp11
{
  template<typename Primary, typename Fallback>
  class fallback
  {
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
     *
     * @return Primary&
     */
    Primary & get_primary() noexcept
    {
      return primary;
    }
    /**
     * @brief Get the primary object
     *
     * @return Primary const&
     */
    Primary const & get_primary() const noexcept
    {
      return primary;
    }
    /**
     * @brief Get the fallback object
     *
     * @return Fallback&
     */
    Fallback & get_fallback() noexcept
    {
      return fallback;
    }
    /**
     * @brief Get the fallback object
     *
     * @return Fallback const&
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