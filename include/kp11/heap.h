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
    using pointer = void *;
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