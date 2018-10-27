#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <memory> // pointer_traits
#include <type_traits>

namespace kp11
{
  /// @private
  namespace resource_traits_detail
  {
    /// @private
    template<typename T, typename Enable = void>
    struct pointer
    {
      using type = void *;
    };
    /// @private
    template<typename T>
    struct pointer<T, std::void_t<typename T::pointer>>
    {
      using type = typename T::pointer;
    };
    /// @private
    template<typename T>
    using pointer_t = typename pointer<T>::type;

    /// @private
    template<typename T, typename Enable = void>
    struct size_type
    {
      using type =
        std::make_unsigned_t<typename std::pointer_traits<pointer_t<T>>::difference_type>;
    };
    /// @private
    template<typename T>
    struct size_type<T, std::void_t<typename T::size_type>>
    {
      using type = typename T::size_type;
    };
    /// @private
    template<typename T>
    using size_type_t = typename size_type<T>::type;

    /// @private
    template<typename T, typename Enable = void>
    struct max_size
    {
      using size_type = size_type_t<T>;
      static constexpr auto value = static_cast<size_type>(std::numeric_limits<size_type>::max());
    };
    /// @private
    template<typename T>
    struct max_size<T, std::void_t<decltype(T::max_size())>>
    {
      using size_type = size_type_t<T>;
      static constexpr auto value = static_cast<size_type>(T::max_size());
    };
    /// @private
    template<typename T>
    static constexpr auto max_size_v = max_size<T>::value;
  };
  /// Provides a standardized way of accessing properties of `Resources`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct resource_traits
  {
    /// `T::pointer` if present otherwise `void *`.
    using pointer = resource_traits_detail::pointer_t<T>;
    /// `T::size_type` if present otherwise `std::size_t`.
    using size_type = resource_traits_detail::size_type_t<T>;
    /// `T::max_size()` if present otherwise `std::numeric_limits<size_type>::max()`.
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return resource_traits_detail::max_size_v<T>;
    }
    /// Calls `T::allocate`.
    static auto allocate(T & x, size_type size, size_type alignment) noexcept
    {
      return x.allocate(size, alignment);
    }
    /// Calls `T::deallocate`.
    static auto deallocate(T & x, pointer ptr, size_type size, size_type alignment) noexcept
    {
      return x.deallocate(ptr, size, alignment);
    }
  };
  /// Checks if `T` meets the `Resource` concept.
  template<typename T, typename Enable = void>
  struct is_resource : std::false_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  /// @private
  template<typename T>
  struct is_resource<T,
    std::void_t<std::enable_if_t<std::is_default_constructible_v<T>>,
      std::enable_if_t<std::is_same_v<typename resource_traits<T>::pointer,
        decltype(std::declval<T>().allocate(std::declval<typename resource_traits<T>::size_type>(),
          std::declval<typename resource_traits<T>::size_type>()))>>,
      decltype(std::declval<T>().deallocate(std::declval<typename resource_traits<T>::pointer>(),
        std::declval<typename resource_traits<T>::size_type>(),
        std::declval<typename resource_traits<T>::size_type>()))>> : std::true_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;

  template<typename R>
  auto has_owner_expressions(R r,
    typename resource_traits<R>::pointer ptr = {},
    typename resource_traits<R>::size_type size = {},
    typename resource_traits<R>::size_type alignment = {},
    bool b = {}) -> decltype(ptr = r[ptr]);
  /// Checks if `T` meets the `Owner` concept.
  template<typename T, typename Enable = void>
  struct is_owner : std::false_type
  {
  };
  /// Checks if `T` meets the `Owner` concept.
  /// @private
  template<typename T>
  struct is_owner<T,
    std::void_t<std::enable_if_t<is_resource_v<T>>,
      decltype(has_owner_expressions(std::declval<T>()))>> : std::true_type
  {
  };
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  constexpr bool is_owner_v = is_owner<T>::value;

  /// Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct owner_traits : public resource_traits<T>
  {
    /// If `owner` has a convertible to `bool` deallocate function then uses that. Otherwise checks
    /// to see if ptr is owned by using `operator[]` before deallocating.
    ///
    /// @param owner Meets the `Owner` concept.
    /// @param ptr Pointer to deallocate if owned by `owner`.
    /// @param size Size in bytes of the memory pointed to by `ptr`.
    /// @param alignment Alignment in bytes of the memory pointed to by `ptr`.
    ///
    /// @returns (success) `true`, owned by `owner`.
    /// @returns (failure) `false`
    static bool deallocate(T & owner,
      typename resource_traits<T>::pointer ptr,
      typename resource_traits<T>::size_type size,
      typename resource_traits<T>::size_type alignment) noexcept
    {
      // It may be trivial for a type to return success or failure in it's deallocate function, if
      // if is then it should do so.
      if constexpr (std::is_convertible_v<bool,
                      decltype(resource_traits<T>::deallocate(owner, ptr, size, alignment))>)
      {
        return resource_traits<T>::deallocate(owner, ptr, size, alignment);
      }
      // If it is not trivial then we can still determine ownership through operator[].
      else
      {
        if (owner[ptr])
        {
          resource_traits<T>::deallocate(owner, ptr, size, alignment);
          return true;
        }
        return false;
      }
    }
  };
  /// @private
  namespace marker_traits_detail
  {
    /// @private
    template<typename T, typename Enable = void>
    struct max_size
    {
      static constexpr auto value = T::size();
    };
    /// @private
    template<typename T>
    struct max_size<T, std::void_t<decltype(T::max_size())>>
    {
      static constexpr auto value = T::max_size();
    };
    /// @private
    template<typename T>
    static constexpr auto max_size_v = max_size<T>::value;
  };
  /// Provides a standardized way of accessing some properties of `Markers`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct marker_traits
  {
    using size_type = typename T::size_type;
    static constexpr auto max_size() noexcept
    {
      return marker_traits_detail::max_size_v<T>;
    }
  };

  /// Checks if `T` meets the `Marker` concept.
  template<typename T, typename Enable = void>
  struct is_marker : std::false_type
  {
  };
  template<typename R>
  auto has_marker_expressions(R r, typename R::size_type i = {}, typename R::size_type n = {})
    -> decltype(R{},
      n = R::size(),
      n = r.count(),
      n = marker_traits<R>::max_size(),
      n = r.max_alloc(),
      i = r.allocate(n),
      r.deallocate(i, n));
  /// Checks if `T` meets the `Marker` concept.
  /// @private
  template<typename T>
  struct is_marker<T, std::void_t<decltype(has_marker_expressions(std::declval<T>()))>>
      : std::true_type
  {
  };
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  constexpr bool is_marker_v = is_marker<T>::value;

}