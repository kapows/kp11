#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <memory> // pointer_traits
#include <type_traits>

namespace kp11
{
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
  auto has_resource_expressions(R r,
    typename R::pointer ptr = {nullptr},
    typename resource_traits<R>::size_type size = {},
    typename resource_traits<R>::size_type alignment = {}) -> decltype(typename R::pointer{nullptr},
    typename resource_traits<R>::size_type{},
    R{},
    size = resource_traits<R>::max_size(),
    ptr = r.allocate(size, alignment),
    r.deallocate(ptr, size, alignment));
  /// Checks if `T` meets the `Resource` concept.
  template<typename T, typename Enable = void>
  struct is_resource : std::false_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  /// @private
  template<typename T>
  struct is_resource<T, std::void_t<decltype(has_resource_expressions(std::declval<T>()))>>
      : std::true_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;

  /// @brief Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct owner_traits
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
  template<typename R>
  auto has_owner_expressions(R r,
    typename resource_traits<R>::pointer ptr = {nullptr},
    typename resource_traits<R>::size_type size = {},
    typename resource_traits<R>::size_type alignment = {},
    bool b = {})
    -> decltype(ptr = r[ptr], b = owner_traits<R>::deallocate(r, ptr, size, alignment));
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
  auto has_marker_expressions(R r, typename R::size_type i = {}, typename R::size_type n = {})
    -> decltype(typename R::size_type{},
      R{},
      n = R::size(),
      n = r.count(),
      n = marker_traits<R>::max_size(),
      n = r.max_alloc(),
      i = r.allocate(n),
      r.deallocate(i, n));
  /// Checks if `T` meets the `Marker` concept.
  template<typename T, typename Enable = void>
  struct is_marker : std::false_type
  {
  };
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

#undef KP11_TRAITS_NESTED_STATIC_FUNC
#undef KP11_TRAITS_NESTED_TYPE
}