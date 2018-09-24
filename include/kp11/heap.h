#pragma once

#include <cstddef> // size_t

namespace kp11
{
  /**
   * @brief Calls `new` on `allocate` and `delete` on `deallocate`.
   *    Meets the requirements of `Resource`.
   */
  class heap
  {
  public: // typedefs
    /**
     * @brief Pointer that `new` and `delete` use
     */
    using pointer = void *;
    /**
     * @brief Size type that `new` and `delete` use
     */
    using size_type = std::size_t;

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept;
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
  };
}