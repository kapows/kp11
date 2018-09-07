#pragma once

#include "traits.h" // is_resource_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <new> // bad alloc

namespace kp11
{
  template<typename T, typename Resource>
  class allocator
  {
    static_assert(is_resource_v<Resource>);

  public: // typedefs
    using value_type = T;
    using pointer =
      typename std::pointer_traits<typename Resource::pointer>::template rebind<value_type>;
    using void_pointer = typename Resource::pointer;
    using size_type = typename Resource::size_type;
    template<typename U>
    struct rebind
    {
      using other = allocator<U, Resource>;
    };

  public: // constructors
    allocator(Resource * resource) noexcept : resource(resource)
    {
    }
    template<typename U>
    allocator(allocator<U, Resource> const & x) noexcept : resource(x.get_resource())
    {
    }

  public: // modifiers
    pointer allocate(size_type n)
    {
      auto ptr = resource->allocate(static_cast<size_type>(sizeof(T) * n), alignof(T));
      if (!ptr)
      {
        throw std::bad_alloc();
      }
      return static_cast<pointer>(ptr);
    }
    void deallocate(pointer ptr, size_type n) noexcept
    {
      resource->deallocate(
        static_cast<void_pointer>(ptr), static_cast<size_type>(sizeof(T) * n), alignof(T));
    }

  public: // accessors
    Resource * get_resource() const noexcept
    {
      return resource;
    }

  private: // variables
    Resource * resource;
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