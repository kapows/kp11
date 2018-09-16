#pragma once

#include "traits.h" // is_resource_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <new> // bad alloc

namespace kp11
{
  /**
   * @brief Adaptor that allows a `Resource` to be used as an allocator
   *
   * @tparam T value type
   * @tparam Resource type that meets the `Resource` concept
   */
  template<typename T, typename Resource>
  class allocator
  {
    static_assert(is_resource_v<Resource>);

  public: // typedefs
    /**
     * @brief value type
     */
    using value_type = T;
    /**
     * @brief pointer type
     */
    using pointer =
      typename std::pointer_traits<typename Resource::pointer>::template rebind<value_type>;
    /**
     * @brief void pointer type
     */
    using void_pointer = typename Resource::pointer;
    /**
     * @brief size type
     */
    using size_type = typename Resource::size_type;
    /**
     * @brief rebind type
     *
     * @tparam U type to rebind to
     */
    template<typename U>
    struct rebind
    {
      /**
       * @brief other type
       */
      using other = allocator<U, Resource>;
    };

  public: // constructors
    /**
     * @brief Construct a new allocator object
     *
     * @param resource pointer to a type that meets the `Resource` concept
     */
    allocator(Resource * resource) noexcept : resource(resource)
    {
    }
    /**
     * @brief Construct a new allocator object
     */
    template<typename U>
    allocator(allocator<U, Resource> const & x) noexcept : resource(x.get_resource())
    {
    }

  public: // modifiers
    /**
     * @brief Allocates memory
     *
     * @param n number of `value_type` objects to allocate for
     * @return pointer to the beginning of allocated memory if successful
     * @exception std::bad_alloc if unsuccessful
     */
    pointer allocate(size_type n)
    {
      auto ptr = resource->allocate(static_cast<size_type>(sizeof(T) * n), alignof(T));
      if (!ptr)
      {
        throw std::bad_alloc();
      }
      return static_cast<pointer>(ptr);
    }
    /**
     * @brief Deallocates memory
     *
     * @param ptr pointer returned by a call to `allocate`
     * @param n corresponding parameter used in the call to `allocate`
     */
    void deallocate(pointer ptr, size_type n) noexcept
    {
      resource->deallocate(
        static_cast<void_pointer>(ptr), static_cast<size_type>(sizeof(T) * n), alignof(T));
    }

  public: // accessors
    /**
     * @brief Get the resource object
     */
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