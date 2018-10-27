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
    pointer allocate(size_type size, size_type alignment) noexcept;
    void deallocate(pointer ptr, size_type size, size_type alignment) noexcept;
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
      std::enable_if_t<std::is_default_constructible_v<T>>,
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
    pointer allocate(size_type size, size_type alignment) noexcept;
    bool deallocate(pointer ptr, size_type size, size_type alignment) noexcept;
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

  /// Provides a way to deallocate owned memory from `owner`s.
  /// @private
  template<typename T, bool Enable = is_owner_v<T>>
  struct owner_traits;
  /// Provides a way to deallocate owned memory from `owner`s.
  template<typename T>
  struct owner_traits<T, true>
  {
    /// Pointer type.
    using pointer = typename T::pointer;
    /// Size type.
    using size_type = typename T::size_type;
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

  /* Marker Exemplar
  class marker
  {
  public:
    using size_type = std::size_t;
    static constexpr size_type max_size() noexcept;
    size_type size() const noexcept;
    size_type biggest() const noexcept;
    size_type allocate(size_type n) noexcept;
    void deallocate(size_type index, size_type n) noexcept;
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
      std::enable_if_t<std::is_same_v<typename T::size_type, decltype(std::declval<T>().size())>>,
      std::enable_if_t<
        std::is_same_v<typename T::size_type, decltype(std::declval<T>().biggest())>>,
      std::enable_if_t<std::is_same_v<typename T::size_type,
        decltype(std::declval<T>().allocate(std::declval<typename T::size_type>()))>>,
      decltype(std::declval<T>().deallocate(std::declval<typename T::size_type>(),
        std::declval<typename T::size_type>()))>> : std::true_type
  {
  };
  /// Checks if `T` meets the `Marker` concept.
  template<typename T>
  constexpr bool is_marker_v = is_marker<T>::value;
}