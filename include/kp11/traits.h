/// @file
#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <memory> // pointer_traits
#include <type_traits>

namespace kp11
{
/// @private
#define KP11_TRAITS_NESTED_TYPE(TYPE, ALT)                     \
private:                                                       \
  template<typename MY_T, typename Enable = void>              \
  struct TYPE##_helper                                         \
  {                                                            \
    using type = ALT;                                          \
  };                                                           \
  template<typename MY_T>                                      \
  struct TYPE##_helper<MY_T, std::void_t<typename MY_T::TYPE>> \
  {                                                            \
    using type = typename MY_T::TYPE;                          \
  };                                                           \
  template<typename MY_T>                                      \
  using TYPE##_helper_t = typename TYPE##_helper<MY_T>::type;  \
                                                               \
public:
/// @private
#define KP11_TRAITS_NESTED_STATIC_FUNC(FUNC)                                       \
private:                                                                           \
  template<typename MY_T, typename Enable = void>                                  \
  struct FUNC##_helper : std::false_type                                           \
  {                                                                                \
  };                                                                               \
  template<typename MY_T>                                                          \
  struct FUNC##_helper<MY_T, std::void_t<decltype(MY_T::FUNC())>> : std::true_type \
  {                                                                                \
  };                                                                               \
  template<typename MY_T>                                                          \
  static constexpr auto FUNC##_helper_v = FUNC##_helper<MY_T>::value;              \
                                                                                   \
public:

  // Detector Idiom

  /// @private
  template<typename Enable, template<typename...> typename T, typename... Args>
  struct detector : std::false_type
  {
  };
  /// @private
  template<template<typename...> typename T, typename... Args>
  struct detector<std::void_t<T<Args...>>, T, Args...> : std::true_type
  {
  };
  /// @private
  template<template<typename...> typename T, typename... Args>
  using is_detected = detector<void, T, Args...>;
  /// @private
  template<template<typename...> typename T, typename... Args>
  inline constexpr auto is_detected_v = is_detected<T, Args...>::value;

  /// @brief Provides a standardized way of accessing properties of `Resources`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct resource_traits
  {
    /// `T::pointer`
    using pointer = typename T::pointer;
    KP11_TRAITS_NESTED_TYPE(
      size_type, std::make_unsigned_t<typename std::pointer_traits<pointer>::difference_type>)
    /// `T::size_type` if present otherwise `std::size_t`.
    using size_type = size_type_helper_t<T>;
    KP11_TRAITS_NESTED_STATIC_FUNC(max_size)
    /// `T::max_size()` if present otherwise `std::numeric_limits<size_type>::%max()`
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_helper_v<T>)
      {
        return T::max_size();
      }
      else
      {
        return std::numeric_limits<size_type>::max();
      }
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
  /// @private
  template<typename R>
  auto IsResource_h(R r,
    typename R::pointer ptr = {nullptr},
    typename resource_traits<R>::size_type size = {},
    typename resource_traits<R>::size_type alignment = {}) -> decltype(typename R::pointer{nullptr},
    typename resource_traits<R>::size_type{},
    R{},
    size = resource_traits<R>::max_size(),
    ptr = r.allocate(size, alignment),
    r.deallocate(ptr, size, alignment));
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  using IsResource = decltype(IsResource_h(std::declval<T>()));
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  using is_resource = is_detected<IsResource, T>;
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  inline constexpr auto is_resource_v = is_resource<T>::value;

  /// @brief Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct owner_traits : public resource_traits<T>
  {
    using pointer = typename resource_traits<T>::pointer;
    using size_type = typename resource_traits<T>::size_type;

    /// Calls `T::operator[]`.
    static decltype(auto) owns(T & owner, pointer ptr) noexcept
    {
      return owner[ptr];
    }
    /// If `owner` has a convertible to `bool` deallocate function then uses that. Otherwise checks
    /// to see if ptr is owned by using `operator[]` before deallocating.
    static bool deallocate(T & owner, pointer ptr, size_type size, size_type alignment) noexcept
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
        if (owns(owner, ptr))
        {
          resource_traits<T>::deallocate(owner, ptr, size, alignment);
          return true;
        }
        return false;
      }
    }
  };
  /// @private
  template<typename R, typename = IsResource<R>>
  auto IsOwner_h(R r,
    typename resource_traits<R>::pointer ptr = {nullptr},
    typename resource_traits<R>::size_type size = {},
    typename resource_traits<R>::size_type alignment = {},
    bool b = {})
    -> decltype(ptr = r[ptr], b = owner_traits<R>::deallocate(r, ptr, size, alignment));
  /// Checks if `T` meets the `Owner` concept.
  template<typename R>
  using IsOwner = decltype(IsOwner_h(std::declval<R>()));
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  using is_owner = is_detected<IsOwner, T>;
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  inline constexpr auto is_owner_v = is_owner<T>::value;

  /// @brief Provides a standardized way of accessing some properties of `Markers`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct marker_traits
  {
    KP11_TRAITS_NESTED_STATIC_FUNC(max_size)
    /// `T::max_size()` if present otherwise `T::size()`.
    static constexpr auto max_size() noexcept
    {
      if constexpr (max_size_helper_v<T>)
      {
        return T::max_size();
      }
      else
      {
        return T::size();
      }
    }
  };
  /// @private
  template<typename R>
  auto IsMarker_h(R r, typename R::size_type i = {}, typename R::size_type n = {})
    -> decltype(typename R::size_type{},
      R{},
      n = R::size(),
      n = r.count(),
      n = marker_traits<R>::max_size(),
      n = r.max_alloc(),
      i = r.allocate(n),
      r.deallocate(i, n));
  /// Checks if `T` meets the `Marker` concept.
  template<typename R>
  using IsMarker = decltype(IsMarker_h(std::declval<R>()));
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  using is_marker = is_detected<IsMarker, T>;
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  inline constexpr auto is_marker_v = is_marker<T>::value;

#undef KP11_TRAITS_NESTED_STATIC_FUNC
#undef KP11_TRAITS_NESTED_TYPE
}