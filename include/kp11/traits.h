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
#define KP11_TRAITS_NESTED_TYPE(TYPE, ALT)                                      \
private:                                                                        \
  template<typename MY_T, typename Enable = void>                               \
  struct TYPE##_picker : std::false_type                                        \
  {                                                                             \
    using type = ALT;                                                           \
  };                                                                            \
  template<typename MY_T>                                                       \
  struct TYPE##_picker<MY_T, std::void_t<typename MY_T::TYPE>> : std::true_type \
  {                                                                             \
    using type = typename MY_T::TYPE;                                           \
  };                                                                            \
  template<typename MY_T>                                                       \
  using TYPE##_picker_t = typename TYPE##_picker<MY_T>::type;                   \
                                                                                \
public:                                                                         \
  template<typename MY_T>                                                       \
  static constexpr auto TYPE##_provided_v = TYPE##_picker<MY_T>::value;

/// @private
#define KP11_TRAITS_NESTED_STATIC_FUNC(FUNC)                                         \
  template<typename MY_T, typename Enable = void>                                    \
  struct FUNC##_provided : std::false_type                                           \
  {                                                                                  \
  };                                                                                 \
  template<typename MY_T>                                                            \
  struct FUNC##_provided<MY_T, std::void_t<decltype(MY_T::FUNC())>> : std::true_type \
  {                                                                                  \
  };                                                                                 \
                                                                                     \
public:                                                                              \
  template<typename MY_T>                                                            \
  static constexpr auto FUNC##_provided_v = FUNC##_provided<MY_T>::value;

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

  template<typename T>
  using remove_cvref_t = std::remove_reference_t<std::remove_cv_t<T>>;

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
    using size_type = size_type_picker_t<T>;
    KP11_TRAITS_NESTED_STATIC_FUNC(max_size)
    /// `T::max_size()` if present otherwise `std::numeric_limits<size_type>::%max()`
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_provided_v<T>)
      {
        return T::max_size();
      }
      else
      {
        return std::numeric_limits<size_type>::max();
      }
    }
    /// Calls `T::allocate`.
    static pointer allocate(T & x, size_type size, size_type alignment) noexcept
    {
      assert(size <= max_size());
      return x.allocate(size, alignment);
    }
    /// Calls `T::deallocate`.
    static decltype(auto) deallocate(
      T & x, pointer ptr, size_type size, size_type alignment) noexcept
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
  /// `Resource` facade
  template<typename T, typename R = remove_cvref_t<T>>
  struct Resource
  {
    static_assert(is_resource_v<R>);

  public: // constructors
    /// Forwarding ctor
    template<typename... Args>
    Resource(Args &&... args) noexcept(std::is_nothrow_constructible_v<R, Args...>) :
        value(std::forward<Args>(args)...)
    {
    }
    /// Deleted because facade shouldn't be copied.
    Resource(Resource const & x) = delete;
    /// Deleted because facade shouldn't be moved.
    Resource(Resource && x) = delete;
    /// Deleted because facade shouldn't be copied.
    Resource & operator=(Resource const &) = delete;
    /// Deleted because facade shouldn't be moved.
    Resource & operator=(Resource &&) = delete;
    /// Forwarding assignment
    template<typename S>
    decltype(auto) operator=(S && rhs)
    {
      value = std::forward<S>(rhs);
      return (value);
    }

  public: // expressions
    /// `T::pointer`
    using pointer = typename R::pointer;
    /// `resource_traits<T>::size_type`
    using size_type = typename resource_traits<R>::size_type;
    /// `resource_traits<T>::max_size`
    static constexpr size_type max_size() noexcept
    {
      return resource_traits<R>::max_size();
    }
    /// `T::allocate`.
    pointer allocate(size_type size, size_type alignment) noexcept
    {
      assert(size <= max_size());
      return value.allocate(size, alignment);
    }
    /// `T::deallocate`.
    decltype(auto) deallocate(pointer ptr, size_type size, size_type alignment) noexcept
    {
      return value.deallocate(ptr, size, alignment);
    }

  public: // variables
    T value;
  };
  template<typename T>
  Resource(T &)->Resource<T &>;
  template<typename T>
  Resource(T const &)->Resource<T const &>;

  /// @brief Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct owner_traits : public resource_traits<T>
  {
    using pointer = typename resource_traits<T>::pointer;
    using size_type = typename resource_traits<T>::size_type;

    /// Calls `T::operator[]`.
    static pointer owns(T & owner, pointer ptr) noexcept
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
  /// `Owner` facade
  template<typename T, typename R = remove_cvref_t<T>>
  struct Owner : public Resource<T>
  {
    static_assert(is_owner_v<R>);

  public: // constructors
    using Resource<T>::Resource;

  public: // typedefs
    /// Pointer type
    using pointer = typename Resource<T>::pointer;
    /// Size type
    using size_type = typename Resource<T>::size_type;

  public: // expressions
    /// `T::operator[]`
    pointer operator[](pointer ptr) noexcept
    {
      return Resource<T>::value[ptr];
    }
    /// `owner_traits<T>::deallocate`
    bool deallocate(pointer ptr, size_type size, size_type alignment) noexcept
    {
      return owner_traits<R>::deallocate(Resource<T>::value, ptr, size, alignment);
    }
  };
  template<typename T>
  Owner(T &)->Owner<T &>;
  template<typename T>
  Owner(T const &)->Owner<T const &>;

  /// @brief Provides a standardized way of accessing some properties of `Markers`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct marker_traits
  {
    using size_type = typename T::size_type;
    KP11_TRAITS_NESTED_STATIC_FUNC(max_size)
    /// `T::max_size()` if present otherwise `T::size()`.
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_provided_v<T>)
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

  /// `Marker` facade
  template<typename T, typename R = remove_cvref_t<T>>
  class Marker
  {
    static_assert(is_marker_v<R>);

  public: // typedefs
    /// `T::size_type`
    using size_type = typename R::size_type;

  public: // constructors
    /// Forwarding ctor
    template<typename... Args>
    Marker(Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
        value(std::forward<Args>(args)...)
    {
    }
    /// Deleted because facade shouldn't be copied.
    Marker(Marker const & x) = delete;
    /// Deleted because facade shouldn't be moved.
    Marker(Marker && x) = delete;
    /// Deleted because facade shouldn't be copied.
    Marker & operator=(Marker const &) = delete;
    /// Deleted because facade shouldn't be moved.
    Marker & operator=(Marker &&) = delete;
    /// Copy assignment
    template<typename S>
    decltype(auto) operator=(S && rhs)
    {
      value = std::forward<S>(rhs);
      return (value);
    }

  public: // accessors
    operator decltype(auto)() noexcept
    {
      return (value);
    }

  public: // concept expressions
    /// `R::size`
    static constexpr size_type size() noexcept
    {
      return R::size();
    }
    /// `R::count`
    size_type count() const noexcept
    {
      auto n = value.count();
      assert(n <= max_size());
      return n;
    }
    /// `marker_traits<R>::max_size`
    static constexpr size_type max_size() noexcept
    {
      return marker_traits<R>::max_size();
    }
    /// `R::max_alloc`.
    size_type max_alloc() const noexcept
    {
      auto n = value.max_alloc();
      assert(n <= max_size());
      assert(n <= size() - count());
      return n;
    }
    /// `R::allocate`
    size_type allocate(size_type n) noexcept
    {
      assert(n <= max_alloc());
      auto i = value.allocate(n);
      assert(i < max_size());
      return i;
    }
    /// `T::deallocate`
    decltype(auto) deallocate(size_type i, size_type n) noexcept
    {
      assert(i < max_size());
      assert(i + n <= max_size());
      return value.deallocate(i, n);
    }

  public: // variables
    T value;
  };
  template<typename T>
  Marker(T &)->Marker<T &>;
  template<typename T>
  Marker(T const &)->Marker<T const &>;

#undef KP11_TRAITS_NESTED_STATIC_FUNC
#undef KP11_TRAITS_NESTED_TYPE
}