#pragma once

#include "traits.h" // is_resource_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <new> // bad_alloc

namespace kp11
{
  /// Resource that `allocator<T,Resource>` uses.
  template<typename Resource>
  static Resource & allocator_singleton()
  {
    static Resource resource;
    return resource;
  }
  /// @brief Adaptor that wraps a `Resource` so that it can be used as an allocator.
  ///
  /// Makes `Resource` a static variable and uses that to allocate/deallocate. Use this when you
  /// want to make a stateless global allocator.
  ///
  /// @tparam T Value type.
  /// @tparam Resource Meets the `Resource` concept.
  template<typename T, typename Resource>
  class allocator
  {
  public: // typedefs
    /// Value type.
    using value_type = T;
    /// Pointer type.
    using pointer =
      typename std::pointer_traits<typename Resource::pointer>::template rebind<value_type>;
    /// Void pointer type.
    using void_pointer = typename Resource::pointer;
    /// Size type.
    using size_type = typename Resource::size_type;
    /// Rebind type.
    template<typename U>
    struct rebind
    {
      using other = allocator<U, Resource>;
    };

  public: // constructors
    /// Defined because of the rebind constructor.
    allocator() noexcept = default;
    /// Rebind constructor.
    template<typename U>
    allocator(allocator<U, Resource> const & x) noexcept
    {
    }

  public: // modifiers
    /// Forwards the request of `sizeof(T) * n` to `Resource::allocate`.
    ///
    /// @param n Number of `sizeof(T)` bytes to allocate.
    ///
    /// @returns Pointer to a memory block of size `sizeof(T) * n` bytes aligned to `alignof(T)`.
    ///
    /// @throws (failure) std::bad_alloc
    pointer allocate(size_type n)
    {
      auto ptr = get_resource().allocate(static_cast<size_type>(sizeof(T) * n), alignof(T));
      if (!ptr)
      {
        throw std::bad_alloc();
      }
      return static_cast<pointer>(ptr);
    }
    /// Forwards the request to deallocate to `Resource::deallocate`.
    ///
    /// @param ptr Pointer to memory returned by `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    void deallocate(pointer ptr, size_type n) noexcept
    {
      get_resource().deallocate(
        static_cast<void_pointer>(ptr), static_cast<size_type>(sizeof(T) * n), alignof(T));
    }

  public: // accessors
    /// @returns Pointer to the `Resource` which was passed into the constructor.
    static Resource & get_resource() noexcept
    {
      return allocator_singleton<Resource>();
    }
  };
  template<typename T, typename U, typename R>
  bool operator==(allocator<T, R> const & lhs, allocator<U, R> const & rhs) noexcept
  {
    return true;
  }
  template<typename T, typename U, typename R>
  bool operator!=(allocator<T, R> const & lhs, allocator<U, R> const & rhs) noexcept
  {
    return false;
  }

  /// @brief Adaptor that wraps a `Resource` so that it can be used as an allocator. This is a
  /// pointer specialization on `Resource`.
  ///
  /// Makes `Resource` a pointer variable and uses that to allocate/deallocate. Use this when you
  /// want to make a stateful local allocator.
  ///
  /// @tparam T Value type.
  /// @tparam Resource Meets the `Resource` concept.
  template<typename T, typename Resource>
  class allocator<T, Resource *>
  {
    static_assert(is_resource_v<Resource>);

  public: // typedefs
    /// Value type.
    using value_type = T;
    /// Pointer type.
    using pointer =
      typename std::pointer_traits<typename Resource::pointer>::template rebind<value_type>;
    /// Void pointer type.
    using void_pointer = typename Resource::pointer;
    /// Size type.
    using size_type = typename Resource::size_type;
    /// Rebind type.
    template<typename U>
    struct rebind
    {
      using other = allocator<U, Resource *>;
    };

  public: // constructors
    /// If `resource` is `nullptr` calling `allocate` or `deallocate` is undefined.
    ///
    /// @param resource Pointer to a `Resource`.
    allocator(Resource * resource) noexcept : resource(resource)
    {
    }
    /// Rebind constructor.
    template<typename U>
    allocator(allocator<U, Resource *> const & x) noexcept : resource(x.get_resource())
    {
    }

  public: // modifiers
    /// Forwards the request of `sizeof(T) * n` to `Resource::allocate`.
    ///
    /// @param n Number of `sizeof(T)` bytes to allocate.
    ///
    /// @returns Pointer to a memory block of size `sizeof(T) * n` bytes aligned to `alignof(T)`.
    ///
    /// @throws (failure) std::bad_alloc
    pointer allocate(size_type n)
    {
      auto ptr = resource->allocate(static_cast<size_type>(sizeof(T) * n), alignof(T));
      if (!ptr)
      {
        throw std::bad_alloc();
      }
      return static_cast<pointer>(ptr);
    }
    /// Forwards the request to deallocate to `Resource::deallocate`.
    ///
    /// @param ptr Pointer to memory returned by `allocate`.
    /// @param n Corresponding parameter in the call to `allocate`.
    void deallocate(pointer ptr, size_type n) noexcept
    {
      resource->deallocate(
        static_cast<void_pointer>(ptr), static_cast<size_type>(sizeof(T) * n), alignof(T));
    }

  public: // accessors
    /// @returns Pointer to the `Resource` which was passed into the constructor.
    Resource * get_resource() const noexcept
    {
      return resource;
    }

  private: // variables
    Resource * resource;
  };
  template<typename T, typename U, typename R>
  bool operator==(allocator<T, R *> const & lhs, allocator<U, R *> const & rhs) noexcept
  {
    return lhs.get_resource() == rhs.get_resource();
  }
  template<typename T, typename U, typename R>
  bool operator!=(allocator<T, R *> const & lhs, allocator<U, R *> const & rhs) noexcept
  {
    return lhs.get_resource() != rhs.get_resource();
  }
}