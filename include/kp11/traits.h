#pragma once

#include <type_traits>

namespace kp11
{
  template<typename T>
  struct is_resource : std::false_type
  {
  };

  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;
}