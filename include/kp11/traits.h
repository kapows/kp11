/// @file
#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <memory> // pointer_traits
#include <type_traits>

namespace kp11
{
/// Defines TYPE_picker that returns T::TYPE if it exists otherwise ALT
/// @param TYPE Type to check for the prescence of.
/// @param ALT Alternative type if TYPE is not present.
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

/// Adds concept forwarding constructor for single variable
#define KP11_CONCEPT_FORWARDING_CTOR(CONCEPT_NAME, PRIVATE_NAME)            \
public:                                                                     \
  template<typename... Args>                                                \
  CONCEPT_NAME(Args &&... args) : PRIVATE_NAME(std::forward<Args>(args)...) \
  {                                                                         \
  }
/// Adds concept class template argument deduction guides
#define KP11_CONCEPT_CTAD(NAME) \
  template<typename... Args>    \
  NAME(Args &...)->NAME<Args &...>;
/// Adds concept variable declarations
#define KP11_CONCEPT_VAR_DECL(TYPE, NAME)                      \
private:                                                       \
  virtual std::remove_reference_t<TYPE> & NAME() noexcept = 0; \
  virtual std::remove_reference_t<TYPE> const & NAME() const noexcept = 0;
/// Adds concept variable definitions
#define KP11_CONCEPT_VAR_DEF(TYPE, NAME, PRIVATE_NAME)        \
private:                                                      \
  TYPE PRIVATE_NAME;                                          \
  std::remove_reference_t<TYPE> & NAME() noexcept             \
  {                                                           \
    return PRIVATE_NAME;                                      \
  };                                                          \
  std::remove_reference_t<TYPE> const & NAME() const noexcept \
  {                                                           \
    return PRIVATE_NAME;                                      \
  };
/// Adds concept variable implicit cast
#define KP11_CONCEPT_VAR_IMPLICIT_CAST(TYPE, NAME) \
public:                                            \
  operator TYPE &() noexcept                       \
  {                                                \
    return NAME();                                 \
  };                                               \
  operator TYPE const &() const noexcept           \
  {                                                \
    return NAME();                                 \
  };

  /// @brief Provides a standardized way of accessing properties of `Resources`.
  /// Autogenerates some things if they are not present.
  template<typename T>
  struct resource_traits
  {
    /// `T::pointer`
    using pointer = typename T::pointer;

    KP11_TRAITS_NESTED_TYPE(
      size_type, std::make_unsigned_t<typename std::pointer_traits<pointer>::difference_type>)
    /// `true` if present otherwise `false`.
    static constexpr auto size_type_present_v = size_type_picker<T>::value;
    /// `T::size_type` if present otherwise `std::size_t`.
    using size_type = typename size_type_picker<T>::type;

  public: // max size detector
    /// @private
    template<typename R>
    static auto MaxSizePresent_h(R r, size_type n = {}) -> decltype(n = R::max_size());
    /// Check if `R::max_size()` is present.
    template<typename R>
    using MaxSizePresent = decltype(MaxSizePresent_h(std::declval<R>()));
    /// Check if `T::max_size()` is present.
    using max_size_present = is_detected<MaxSizePresent, T>;

  public:
    /// `true` if present otherwise `false`.
    static constexpr auto max_size_present_v = max_size_present::value;
    /// `T::max_size()` if present otherwise `std::numeric_limits<size_type>::%max()`
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_present_v)
      {
        return T::max_size();
      }
      else
      {
        return std::numeric_limits<size_type>::max();
      }
    }
  };
  /// Something
  template<typename R,
    typename = typename R::pointer,
    typename = typename resource_traits<R>::size_type>
  auto IsResource_h(R r,
    typename R::pointer ptr = {nullptr},
    typename resource_traits<R>::size_type size = {},
    typename resource_traits<R>::size_type alignment = {}) -> decltype(R{},
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
  /// Provides a standardized way of accessing properties of `Resources`.
  template<typename T>
  class ResourceConcept
  {
    static_assert(is_resource_v<T>);
    KP11_CONCEPT_VAR_DECL(T, value)

  public: // expressions
    /// `T::pointer`
    using pointer = typename T::pointer;
    /// `resource_traits<T>::%size_type`
    using size_type = typename resource_traits<T>::size_type;
    /// `resource_traits<T>::%max_size`
    static constexpr size_type max_size() noexcept
    {
      return resource_traits<T>::max_size();
    }
    /// `T::allocate`
    pointer allocate(size_type size, size_type alignment) noexcept
    {
      assert(size <= max_size());
      return value().allocate(size, alignment);
    }
    /// `T::deallocate`
    decltype(auto) deallocate(pointer ptr, size_type size, size_type alignment) noexcept
    {
      return value().deallocate(ptr, size, alignment);
    }
  };
  template<typename T, typename R = std::remove_reference_t<T>>
  class Resource : public ResourceConcept<R>
  {
    KP11_CONCEPT_VAR_DEF(T, value, my_value)
    KP11_CONCEPT_VAR_IMPLICIT_CAST(T, value)
    KP11_CONCEPT_FORWARDING_CTOR(Resource, my_value)
  };
  KP11_CONCEPT_CTAD(Resource)

  /// @brief Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not present.
  template<typename T>
  struct owner_traits : public resource_traits<T>
  {
    using typename resource_traits<T>::pointer;
    using typename resource_traits<T>::size_type;

    /// If `owner` has a convertible to `bool` deallocate function then uses that. Otherwise checks
    /// to see if ptr is owned by using `operator[]` before deallocating.
    static bool deallocate(T & owner, pointer ptr, size_type size, size_type alignment) noexcept
    {
      // It may be trivial for a type to return success or failure in it's deallocate function, if
      // if is then it should do so.
      if constexpr (std::is_convertible_v<bool, decltype(owner.deallocate(ptr, size, alignment))>)
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
  /// Provides a standardized way of accessing properties of `Owners`.
  template<typename T>
  class OwnerConcept : public ResourceConcept<T>
  {
    static_assert(is_owner_v<T>);
    KP11_CONCEPT_VAR_DECL(T, value)

  public: // typedefs
    using typename ResourceConcept<T>::pointer;
    using typename ResourceConcept<T>::size_type;

  public: // expressions
    /// `T::operator[]`
    pointer operator[](pointer ptr) noexcept
    {
      return value()[ptr];
    }
    /// `owner_traits<T>::%deallocate`
    bool deallocate(pointer ptr, size_type size, size_type alignment) noexcept
    {
      return owner_traits<T>::deallocate(value(), ptr, size, alignment);
    }
  };
  template<typename T, typename R = std::remove_reference_t<T>>
  class Owner : public OwnerConcept<R>
  {
    KP11_CONCEPT_VAR_DEF(T, value, my_value)
    KP11_CONCEPT_VAR_IMPLICIT_CAST(T, value)
    KP11_CONCEPT_FORWARDING_CTOR(Owner, my_value)
  };
  KP11_CONCEPT_CTAD(Owner)

  /// @brief Provides a standardized way of accessing some properties of `Markers`.
  /// Autogenerates some things if they are not present.
  template<typename T>
  struct marker_traits
  {
    /// `T::size_type`
    using size_type = typename T::size_type;

  public: // max_size detector
    /// @private
    template<typename R>
    static auto MaxSizePresent_h(R r, size_type n = {}) -> decltype(n = R::max_size());
    /// Check if `R::max_size()` is present.
    template<typename R>
    using MaxSizePresent = decltype(MaxSizePresent_h(std::declval<R>()));
    /// Check if `T::max_size()` is present.
    using max_size_present = is_detected<MaxSizePresent, T>;

  public:
    /// `true` if present otherwise `false`.
    static inline constexpr auto max_size_present_v = max_size_present::value;
    /// `T::max_size()` if present otherwise `T::size()`.
    static constexpr size_type max_size() noexcept
    {
      if constexpr (max_size_present_v)
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

  /// Provides a standardized way of accessing some properties of `Markers`.
  template<typename T>
  class MarkerConcept
  {
    static_assert(is_marker_v<T>);
    KP11_CONCEPT_VAR_DECL(T, value)

  public: // typedefs
    /// `T::size_type`
    using size_type = typename T::size_type;

  public: // concept expressions
    /// `T::size`
    static constexpr size_type size() noexcept
    {
      return T::size();
    }
    /// `T::count`
    size_type count() const noexcept
    {
      auto n = value().count();
      assert(n <= max_size());
      return n;
    }
    /// `marker_traits<T>::%max_size`
    static constexpr size_type max_size() noexcept
    {
      return marker_traits<T>::max_size();
    }
    /// `T::allocate`
    size_type allocate(size_type n) noexcept
    {
      assert(n <= max_size());
      auto i = value().allocate(n);
      assert(i < max_size());
      return i;
    }
    /// `T::deallocate`
    decltype(auto) deallocate(size_type i, size_type n) noexcept
    {
      assert(i < max_size());
      assert(i + n <= max_size());
      return value().deallocate(i, n);
    }
  };
  template<typename T, typename R = std::remove_reference_t<T>>
  class Marker : public MarkerConcept<R>
  {
    KP11_CONCEPT_VAR_DEF(T, value, my_value)
    KP11_CONCEPT_VAR_IMPLICIT_CAST(T, value)
    KP11_CONCEPT_FORWARDING_CTOR(Marker, my_value)
  };
  KP11_CONCEPT_CTAD(Marker)

#undef KP11_TRAITS_NESTED_TYPE
}