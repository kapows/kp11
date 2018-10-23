#include "heap.h"

#include <new>

namespace kp11
{
  typename heap::pointer heap::allocate(size_type size, size_type alignment) noexcept
  {
    return ::operator new(size, std::align_val_t(alignment), std::nothrow);
  }
  void heap::deallocate(pointer ptr, size_type size, size_type alignment) noexcept
  {
    ::operator delete(ptr, size, std::align_val_t(alignment));
  }
}