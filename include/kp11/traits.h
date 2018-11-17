/// @file
#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <memory> // pointer_traits
#include <type_traits>

namespace kp11
{
/// @private
#define Enable(BOOL)          \
  std::enable_if_t<BOOL, int> \
  {                           \
  }
/// @private
#define Conv(EXPR, TYPE) Enable((std::is_convertible_v<decltype(EXPR), TYPE>))
/// @private
#define Same(EXPR, TYPE) Enable((std::is_same_v<decltype(EXPR), TYPE>))
/// @private
#define Noexcept(EXPR) Enable(noexcept(EXPR))
/// @private
#define NoexceptConv(EXPR, TYPE) \
  Enable((std::is_convertible_v<decltype(EXPR), TYPE> && noexcept(EXPR)))
/// @private
#define NoexceptSame(EXPR, TYPE) Enable((std::is_same_v<decltype(EXPR), TYPE> && noexcept(EXPR)))

/// @private
#define KP11_TRAITS_NESTED_TYPE(TYPE, ALT)                                          \
  template<typename MY_T, typename Enable = void>                                   \
  struct TYPE##_provided_h : std::false_type                                        \
  {                                                                                 \
    using type = ALT;                                                               \
  };                                                                                \
  template<typename MY_T>                                                           \
  struct TYPE##_provided_h<MY_T, std::void_t<typename MY_T::TYPE>> : std::true_type \
  {                                                                                 \
    using type = typename MY_T::TYPE;                                               \
  };

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
  private: // typedefs
    using pointer = typename T::pointer;

  public: // size_type
    KP11_TRAITS_NESTED_TYPE(
      size_type, std::make_unsigned_t<typename std::pointer_traits<pointer>::difference_type>)
    /// Check if `T` provides a `size_type` type.
    using size_type_provided = size_type_provided_h<T>;
    /// Check if `T` provides a `size_type` type.
    static constexpr auto size_type_present_v = size_type_provided::value;
    /// `T::size_type` if provided otherwise `std::size_t`.
    using size_type = typename size_type_provided::type;

  public: // max_size
    /// @private
    template<typename R>
    static auto MaxSizeProvided_h(R & r) -> decltype(NoexceptSame(R::max_size(), size_type));
    /// Check if `R` provides the proper max_size function.
    template<typename R>
    using MaxSizeProvided = decltype(MaxSizeProvided_h(std::declval<R &>()));
    /// Check if `T` provides the proper max_size function.
    using max_size_provided = is_detected<MaxSizeProvided, T>;
    /// Check if `T` provides the proper max_size function.
    static constexpr auto max_size_provided_v = max_size_provided::value;
    /// `T::max_size()` if provided otherwise `std::numeric_limits<size_type>::%max()`
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_provided_v)
      {
        return T::max_size();
      }
      else
      {
        return std::numeric_limits<size_type>::max();
      }
    }
  };
  /// @private
  template<typename R,
    typename pointer = typename R::pointer,
    typename size_type = typename resource_traits<R>::size_type>
  auto IsResource_h(R & r, pointer ptr = {nullptr}, size_type size = {}, size_type alignment = {})
    -> decltype(Noexcept(R{}),
      NoexceptSame(resource_traits<R>::max_size(), size_type),
      NoexceptSame(r.allocate(size, alignment), pointer),
      Noexcept(r.deallocate(ptr, size, alignment)));
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  using IsResource = decltype(IsResource_h(std::declval<T &>()));
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  using is_resource = is_detected<IsResource, T>;
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  inline constexpr auto is_resource_v = is_resource<T>::value;

  /// @brief Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not present.
  template<typename T>
  struct owner_traits : public resource_traits<T>
  {
  private: // typedefs
    using pointer = typename T::pointer;
    using size_type = typename resource_traits<T>::size_type;

  public: // deallocate
    /// @private
    template<typename R>
    static auto DeallocateProvided_h(
      R & r, pointer ptr = nullptr, size_type size = {}, size_type alignment = {})
      -> decltype(NoexceptConv(r.deallocate(ptr, size, alignment), bool));
    /// Check if `R` provides the proper deallocate function.
    template<typename R>
    using DeallocateProvided = decltype(DeallocateProvided_h(std::declval<R &>()));
    /// Check if `T` provides the proper deallocate function.
    using deallocate_provided = is_detected<DeallocateProvided, T>;
    /// Check if `T` provides the proper deallocate function.
    static constexpr auto deallocate_provided_v = deallocate_provided::value;
    /// If `owner` has a convertible to `bool` deallocate function then uses that. Otherwise checks
    /// to see if ptr is owned by using `operator[]` before deallocating.
    static bool deallocate(T & owner, pointer ptr, size_type size, size_type alignment) noexcept
    {
      // It may be trivial for a type to return success or failure in it's deallocate function, if
      // if is then it should do so.
      if constexpr (deallocate_provided_v)
      {
        return owner.deallocate(ptr, size, alignment);
      }
      // If it is not trivial then we can still determine ownership through operator[].
      else
      {
        if (owner[ptr])
        {
          owner.deallocate(ptr, size, alignment);
          return true;
        }
        return false;
      }
    }
  };
  /// @private
  template<typename R,
    typename = IsResource<R>,
    typename pointer = typename R::pointer,
    typename size_type = typename resource_traits<R>::size_type>
  auto IsOwner_h(R & r, pointer ptr = nullptr, size_type size = {}, size_type alignment = {})
    -> decltype(NoexceptSame(r[ptr], pointer),
      NoexceptConv(owner_traits<R>::deallocate(r, ptr, size, alignment), bool));
  /// Checks if `T` meets the `Owner` concept.
  template<typename R>
  using IsOwner = decltype(IsOwner_h(std::declval<R &>()));
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  using is_owner = is_detected<IsOwner, T>;
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  inline constexpr auto is_owner_v = is_owner<T>::value;

  /// @brief Provides a standardized way of accessing some properties of `Markers`.
  /// Autogenerates some things if they are not present.
  template<typename T>
  struct marker_traits
  {
  private: // typedefs
    using size_type = typename T::size_type;

  public: // max_size
    /// @private
    template<typename R>
    static auto MaxSizeProvided_h(R & r) -> decltype(NoexceptSame(R::max_size(), size_type));
    /// Check if `R` provides the proper max_size function.
    template<typename R>
    using MaxSizeProvided = decltype(MaxSizeProvided_h(std::declval<R &>()));
    /// Check if `T` provides the proper max_size function.
    using max_size_provided = is_detected<MaxSizeProvided, T>;
    /// Check if `T` provides the proper max_size function.
    static constexpr auto max_size_provided_v = max_size_provided::value;
    /// `T::max_size()` if present otherwise `T::size()`
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_provided_v)
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
  template<typename R, typename size_type = typename R::size_type>
  auto IsMarker_h(R & r, size_type i = {}, size_type n = {}) -> decltype(Noexcept(R{}),
    NoexceptSame(R::size(), size_type),
    NoexceptSame(r.count(), size_type),
    NoexceptSame(marker_traits<R>::max_size(), size_type),
    NoexceptSame(r.allocate(n), size_type),
    Noexcept(r.deallocate(i, n)));
  /// Checks if `T` meets the `Marker` concept.
  template<typename R>
  using IsMarker = decltype(IsMarker_h(std::declval<R &>()));
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  using is_marker = is_detected<IsMarker, T>;
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  inline constexpr auto is_marker_v = is_marker<T>::value;

#undef KP11_TRAITS_NESTED_TYPE
#undef Conv
#undef Same
#undef Noexcept
#undef NoexceptConv
#undef NoexceptSame
}