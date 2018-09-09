#pragma once

#include "traits.h" // is_strategy_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <type_traits> // aligned_storage_t

namespace kp11
{
  /**
   * @brief Allocates from a buffer inside itself. Can only allocate once.
   *
   * @tparam Bytes size of buffer
   * @tparam Alignment alignment of buffer
   */
  template<std::size_t Bytes, std::size_t Alignment>
  class local
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  private: // typedefs
    using buffer_type = std::aligned_storage_t<Bytes, Alignment>;
    using buffer_pointer = typename std::pointer_traits<pointer>::template rebind<buffer_type>;
    using buffer_pointer_traits = std::pointer_traits<buffer_pointer>;

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (!allocated && bytes <= Bytes)
      {
        allocated = true;
        return static_cast<pointer>(&buffer);
      }
      return nullptr;
    }
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      allocated = false;
    }

  private: // variables
    bool allocated = false;
    buffer_type buffer;
  };
}