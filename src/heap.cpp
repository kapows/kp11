#include "heap.h"

namespace kp11
{
  typename heap::pointer heap::allocate(size_type bytes, size_type alignment) noexcept
  {
    return nullptr;
  }
  void heap::deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept {}
}