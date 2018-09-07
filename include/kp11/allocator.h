#pragma once

#include <cstddef> // size_t
#include <memory> // pointer_traits
namespace kp11
{
  template<typename T, typename Resource>
  class allocator
  {
  public: // typedefs
    using value_type = T;
    using pointer =
      typename std::pointer_traits<typename Resource::pointer>::template rebind<value_type>;
    using size_type = typename Resource::size_type;
    template<typename U>
    struct rebind
    {
      using other = allocator<U, Resource>;
    };

  public: // constructors
    allocator(Resource * resource) noexcept
    {
    }
    template<typename U>
    allocator(allocator<U, Resource> const &) noexcept
    {
    }

  public: // modifiers
    value_type * allocate(std::size_t n)
    {
      return nullptr;
    }
    void deallocate(value_type * ptr, std::size_t) noexcept
    {
    }

  public: // accessors
    Resource * get_resource() const noexcept
    {
      return nullptr;
    }
  };
  template<typename T, typename U, typename R>
  bool operator==(allocator<T, R> const & lhs, allocator<U, R> const & rhs) noexcept
  {
    return lhs.get_resource() == rhs.get_resource();
  }
  template<typename T, typename U, typename R>
  bool operator!=(allocator<T, R> const & lhs, allocator<U, R> const & rhs) noexcept
  {
    return lhs.get_resource() != rhs.get_resource();
  }
}