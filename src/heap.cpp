#include "heap.h"

#include <new>

namespace kp11
{
  typename heap::pointer heap::allocate(size_type bytes, size_type alignment) noexcept
  {
    return ::operator new(bytes, std::align_val_t(alignment), std::nothrow);
  }
  void heap::deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
  {
    ::operator delete(ptr, bytes, std::align_val_t(alignment));
  }
}