#pragma once

#include <cstddef> // size_t

namespace kp11
{
  template<std::size_t Bytes, std::size_t Alignment, typename Strategy>
  class local
  {
  public: // typedefs
    /**
     * @brief Pointer type
     */
    using pointer = void *;
    /**
     * @brief Size type
     */
    using size_type = std::size_t;

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      return nullptr;
    }
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }
  };

}