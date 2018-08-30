#pragma once

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <type_traits> // aligned_storage_t

namespace kp11
{
  template<std::size_t BlockSize, std::size_t Alignment, typename Marker>
  class free_block
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  private: // typedefs
    using block_type = std::aligned_storage_t<BlockSize, Alignment>;
    using block_pointer = typename std::pointer_traits<pointer>::template rebind<block_type>;

  public: // constructor
    free_block(pointer ptr, size_type bytes, size_type alignment) :
        ptr(static_cast<block_pointer>(ptr))
    {
    }

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (auto i = marker.set(size_from(bytes)); i != marker.size())
      {
        return static_cast<pointer>(&ptr[i]);
      }
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (ptr != nullptr)
      {
        marker.reset(index_from(ptr), size_from(bytes));
      }
    }

  private: // Marker helper functions
    constexpr typename Marker::size_type size_from(size_type bytes) noexcept
    {
      return static_cast<typename Marker::size_type>(bytes / BlockSize);
    }
    typename Marker::size_type index_from(pointer ptr) noexcept
    {
      return static_cast<typename Marker::size_type>(static_cast<block_pointer>(ptr) - this->ptr);
    }

  private: // variables
    Marker marker;
    block_pointer ptr;
  };
}