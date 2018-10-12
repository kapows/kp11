#pragma once

#include <type_traits>

namespace kp11
{
  /* Resource Exemplar
  class resource
  {
  public:
    using pointer = void *;
    using size_type = std::size_t;
    pointer allocate(size_type bytes, size_type alignment) noexcept;
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
  };
  */
  /// Checks if `T` meets the `Resource` concept.
  template<typename T, typename Enable = void>
  struct is_resource : std::false_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  /// @private
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
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;

  /* Owner Exemplar
  class owner
  {
  public:
    using pointer = void *;
    using size_type = std::size_t;
    pointer allocate(size_type bytes, size_type alignment) noexcept;
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
    pointer operator[](pointer ptr) const noexcept;
  };
  */
  /// Checks if `T` meets the `Owner` concept.
  template<typename T, typename Enable = void>
  struct is_owner : std::false_type
  {
  };

  /// Checks if `T` meets the `Owner` concept.
  /// @private
  template<typename T>
  struct is_owner<T,
    std::enable_if_t<is_resource_v<T> &&
                     std::is_same_v<typename T::pointer,
                       decltype(std::declval<T>()[std::declval<typename T::pointer>()])>>>
      : std::true_type
  {
  };
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  constexpr bool is_owner_v = is_owner<T>::value;

  /* Marker Exemplar
  class marker
  {
  public:
    using size_type = std::size_t;
    static constexpr size_type size() noexcept;
    size_type set(size_type n) noexcept;
    void reset(size_type index, size_type n) noexcept;
  };
  */
  /// Checks if `T` meets the `Marker` concept.
  template<typename T, typename Enable = void>
  struct is_marker : std::false_type
  {
  };
  /// Checks if `T` meets the `Marker` concept.
  /// @private
  template<typename T>
  struct is_marker<T,
    std::void_t<typename T::size_type,
      std::enable_if_t<std::is_same_v<typename T::size_type, decltype(T::max_size())>>,
      std::enable_if_t<std::is_same_v<typename T::size_type,
        decltype(std::declval<T>().set(std::declval<typename T::size_type>()))>>,
      decltype(std::declval<T>().reset(std::declval<typename T::size_type>(),
        std::declval<typename T::size_type>()))>> : std::true_type
  {
  };
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  constexpr bool is_marker_v = is_marker<T>::value;
}