#pragma once

#include <cstddef>

namespace kp11
{
  template<std::size_t BlockSize, std::size_t Alignment, typename Marker>
  class free_block
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;
    free_block(pointer ptr, size_type bytes, size_type alignment)
    {
    }
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }
  };
}