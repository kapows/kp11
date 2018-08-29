#pragma once

#include <type_traits>

namespace kp11
{
  /**
   * @brief Check if `T` meets the requiements of `Resource`
   *
   * @tparam T Type to check
   */
  template<typename T, typename Enable = void>
  struct is_resource : std::false_type
  {
  };
  /**
   * @private
   */
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
  /**
   * @brief Helper variable template for `is_resource`
   *
   * @tparam T Type to check
   */
  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;
}