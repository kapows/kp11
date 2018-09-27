#pragma once

#include <type_traits>

namespace kp11
{
  /// Check if `T` meets the `Resource` concept.
  template<typename T, typename Enable = void>
  struct is_resource : std::false_type
  {
  };
  /// Check if `T` meets the `Resource` concept.
  template<typename T>
  struct is_resource<T,
    std::void_t<typename T::pointer,
      typename T::size_type,
      std::enable_if_t<std::is_same_v<typename T::pointer,
        decltype(std::declval<T>().allocate(
          std::declval<typename T::size_type>(), std::declval<typename T::size_type>()))>>,
      decltype(std::declval<T>().deallocate(std::declval<typename T::pointer>(),
        std::declval<typename T::size_type>(),
        std::declval<typename T::size_type>()))>> : std::true_type
  {
  };
  /// Check if `T` meets the `Resource` concept.
  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;

  /// Check if `T` meets the `Marker` concept.
  template<typename T, typename Enable = void>
  struct is_marker : std::false_type
  {
  };
  /// Check if `T` meets the `Marker` concept.
  template<typename T>
  struct is_marker<T,
    std::void_t<typename T::size_type,
      std::enable_if_t<std::is_same_v<typename T::size_type, decltype(std::declval<T>().size())>>,
      std::enable_if_t<std::is_same_v<typename T::size_type,
        decltype(std::declval<T>().set(std::declval<typename T::size_type>()))>>,
      decltype(std::declval<T>().reset(std::declval<typename T::size_type>(),
        std::declval<typename T::size_type>()))>> : std::true_type
  {
  };
  /// Check if `T` meets the `Marker` concept.
  template<typename T>
  constexpr bool is_marker_v = is_marker<T>::value;
}