#pragma once

#include "traits.h" // is_resource_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <new> // bad_alloc

namespace kp11
{
  /// @private
  namespace allocator_detail
  {
    /// @private
    /// @tparam T Value type.
    /// @tparam R Meets the `Resource` concept.
    template<typename T, typename R>
    class base
    {
    public: // typedefs
      /// Value type.
      using value_type = T;
      /// Pointer type.
      using pointer =
        typename std::pointer_traits<typename R::pointer>::template rebind<value_type>;
      /// Void pointer type.
      using void_pointer = typename R::pointer;
      /// Size type.
      using size_type = typename R::size_type;

    public: // capacity
      /// @returns The maximum allocation size supported.
      static constexpr size_type max_size() noexcept
      {
        return R::max_size();
      }

    public: // modifiers
      /// Calls `Resource::allocate` with `sizeof(T) * n` as size and `align(T)` as alignment.
      ///
      /// @param n Number of `sizeof(T)` blocks to allocate.
      ///
      /// @returns Pointer to a memory block of size `sizeof(T) * n` bytes aligned to `alignof(T)`.
      ///
      /// @throws (failure) std::bad_alloc
      pointer allocate(size_type n)
      {
        auto ptr = resource().allocate(static_cast<size_type>(sizeof(T) * n), alignof(T));
        if (!ptr)
        {
          throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
      }
      /// Calls `Resource::deallocate` with `ptr`, `sizeof(T) * n` as size and `align(T)` as
      /// alignment.
      ///
      /// @param ptr Pointer to memory returned by `allocate`.
      /// @param n Corresponding parameter in the call to `allocate`.
      void deallocate(pointer ptr, size_type n) noexcept
      {
        resource().deallocate(
          static_cast<void_pointer>(ptr), static_cast<size_type>(sizeof(T) * n), alignof(T));
      }

    private: // accessors
      virtual R & resource() noexcept = 0;
    };
  }
  /// Resource that `allocator<T, Resource>` uses.
  template<typename Resource>
  static Resource & resource_singleton()
  {
    static Resource resource;
    return resource;
  }
  /// @brief Adaptor that uses a `resource_singleton<Resource>`.
  ///
  /// Use this when you want to make a stateless global allocator.
  ///
  /// @tparam T Value type.
  /// @tparam Resource Meets the `Resource` concept.
  template<typename T, typename Resource>
  class allocator : public allocator_detail::base<T, Resource>
  {
  public: // typedefs
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

  private: // accessors
    virtual Resource & resource() noexcept override
    {
      return resource_singleton<Resource>();
    }
  };
  template<typename T, typename U, typename R>
  constexpr bool operator==(allocator<T, R> const &, allocator<U, R> const &) noexcept
  {
    return true;
  }
  template<typename T, typename U, typename R>
  constexpr bool operator!=(allocator<T, R> const &, allocator<U, R> const &) noexcept
  {
    return false;
  }

  /// @brief Adaptor that stores a `Resource` as a pointer.
  ///
  /// Use this when you want to make a stateful local allocator.
  ///
  /// @tparam T Value type.
  /// @tparam Resource Meets the `Resource` concept.
  template<typename T, typename Resource>
  class allocator<T, Resource *> : public allocator_detail::base<T, Resource>
  {
    static_assert(is_resource_v<Resource>);

  public: // typedefs
    /// Rebind type.
    template<typename U>
    struct rebind
    {
      using other = allocator<U, Resource *>;
    };

  public: // constructors
    /// @param resource Pointer to a `Resource`.
    allocator(Resource * resource) noexcept : my_resource(resource)
    {
      assert(resource != nullptr);
    }
    /// Rebind constructor.
    template<typename U>
    allocator(allocator<U, Resource *> const & x) noexcept : my_resource(x.get_resource())
    {
    }

  private: // accessors
    virtual Resource & resource() noexcept override
    {
      return *my_resource;
    }

  public: // accessors
    /// @returns Pointer to the `Resource` which was passed into the constructor.
    Resource * get_resource() const noexcept
    {
      return my_resource;
    }

  private: // variables
    Resource * my_resource;
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