#pragma once

#include <cstddef> // size_t

namespace kp11
{
  template<std::size_t Bytes,
    std::size_t Alignment,
    typename Strategy,
    typename Resource,
    std::size_t Size = 0>
  class cascade
  {
  public: // typedefs
    using pointer = void *;
    using size_type = std::size_t;

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }

  public: // iterators
    class iterator
    {
    public:
      bool operator==(iterator const & rhs) const noexcept
      {
        return false;
      }
      bool operator!=(iterator const & rhs) const noexcept
      {
        return false;
      }
    };
    class const_iterator
    {
    public:
      bool operator==(const_iterator const & rhs) const noexcept
      {
        return false;
      }
      bool operator!=(const_iterator const & rhs) const noexcept
      {
        return false;
      }
    };
    iterator begin() noexcept
    {
      return {};
    }
    iterator end() noexcept
    {
      return {};
    }
    const_iterator begin() const noexcept
    {
      return {};
    }
    const_iterator end() const noexcept
    {
      return {};
    }
    const_iterator cbegin() const noexcept
    {
      return begin();
    }
    const_iterator cend() const noexcept
    {
      return end();
    }

  public: // element access
    iterator find(pointer ptr) noexcept
    {
      return end();
    }
    const_iterator find(pointer ptr) const noexcept
    {
      return end();
    }
  };
}
