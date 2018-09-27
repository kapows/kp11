#pragma once

#include "traits.h" // is_strategy_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <type_traits> // aligned_storage_t

namespace kp11
{
  /// Allocates from a buffer inside itself.
  /// Can only allocate one memory block at a time.
  /// `Pointer` pointer type
  /// `Size` size type
  /// `Bytes` is the size of the buffer
  /// `Alignment` is the alignment of the buffer
  template<typename Pointer, typename SizeType, std::size_t Bytes, std::size_t Alignment>
  class basic_local
  {
  public: // typedefs
    using pointer = Pointer;
    using size_type = SizeType;

  private: // typedefs
    using buffer_type = std::aligned_storage_t<Bytes, Alignment>;
    using buffer_pointer = typename std::pointer_traits<pointer>::template rebind<buffer_type>;
    using buffer_pointer_traits = std::pointer_traits<buffer_pointer>;

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (!allocated && bytes <= Bytes)
      {
        allocated = true;
        return static_cast<pointer>(&buffer);
      }
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      allocated = false;
    }

  private: // variables
    bool allocated = false;
    buffer_type buffer;
  };

  template<std::size_t Bytes, std::size_t Alignment>
  using local = basic_local<void *, std::size_t, Bytes, Alignment>;
}