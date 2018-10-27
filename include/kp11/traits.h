#pragma once

#include <cstddef> // size_t
#include <limits> // numeric_limits
#include <type_traits>

namespace kp11
{
  /// @private
  namespace resource_traits_detail
  {
    /// @private
    template<typename T, typename Enable = void>
    struct pointer
    {
      using type = void *;
    };
    /// @private
    template<typename T>
    struct pointer<T, std::void_t<typename T::pointer>>
    {
      using type = typename T::pointer;
    };
    /// @private
    template<typename T>
    using pointer_t = typename pointer<T>::type;

    /// @private
    template<typename T, typename Enable = void>
    struct size_type
    {
      using type = std::size_t;
    };
    /// @private
    template<typename T>
    struct size_type<T, std::void_t<typename T::size_type>>
    {
      using type = typename T::size_type;
    };
    /// @private
    template<typename T>
    using size_type_t = typename size_type<T>::type;

    /// @private
    template<typename T, typename Enable = void>
    struct max_size
    {
      using size_type = size_type_t<T>;
      static constexpr auto value = static_cast<size_type>(std::numeric_limits<size_type>::max());
    };
    /// @private
    template<typename T>
    struct max_size<T, std::void_t<decltype(T::max_size())>>
    {
      using size_type = size_type_t<T>;
      static constexpr auto value = static_cast<size_type>(T::max_size());
    };
    /// @private
    template<typename T>
    static constexpr auto max_size_v = max_size<T>::value;
  };
  /// Provides a standardized way of accessing properties of `Resources`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct resource_traits
  {
    /// `T::pointer` if present otherwise `void *`.
    using pointer = resource_traits_detail::pointer_t<T>;
    /// `T::size_type` if present otherwise `std::size_t`.
    using size_type = resource_traits_detail::size_type_t<T>;
    /// `T::max_size()` if present otherwise `std::numeric_limits<size_type>::max()`.
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return resource_traits_detail::max_size_v<T>;
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
  /// Checks if `T` meets the `Resource` concept.
  template<typename T, typename Enable = void>
  struct is_resource : std::false_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  /// @private
  template<typename T>
  struct is_resource<T,
    std::void_t<std::enable_if_t<std::is_default_constructible_v<T>>,
      std::enable_if_t<std::is_same_v<typename resource_traits<T>::pointer,
        decltype(std::declval<T>().allocate(std::declval<typename resource_traits<T>::size_type>(),
          std::declval<typename resource_traits<T>::size_type>()))>>,
      decltype(std::declval<T>().deallocate(std::declval<typename resource_traits<T>::pointer>(),
        std::declval<typename resource_traits<T>::size_type>(),
        std::declval<typename resource_traits<T>::size_type>()))>> : std::true_type
  {
  };
  /// Checks if `T` meets the `Resource` concept.
  template<typename T>
  constexpr bool is_resource_v = is_resource<T>::value;

  /// Checks if `T` meets the `Owner` concept.
  template<typename T, typename Enable = void>
  struct is_owner : std::false_type
  {
  };
  /// Checks if `T` meets the `Owner` concept.
  /// @private
  template<typename T>
  struct is_owner<T,
    std::enable_if_t<
      is_resource_v<T> &&
      std::is_same_v<typename resource_traits<T>::pointer,
        decltype(std::declval<T>()[std::declval<typename resource_traits<T>::pointer>()])>>>
      : std::true_type
  {
  };
  /// Checks if `T` meets the `Owner` concept.
  template<typename T>
  constexpr bool is_owner_v = is_owner<T>::value;

  /// Provides a standardized way of accessing properties of `Owners`.
  /// Autogenerates some things if they are not provided.
  template<typename T>
  struct owner_traits : public resource_traits<T>
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
      std::enable_if_t<std::is_default_constructible_v<T>>,
      std::enable_if_t<std::is_same_v<typename T::size_type, decltype(T::size())>>,
      std::enable_if_t<std::is_same_v<typename T::size_type, decltype(std::declval<T>().count())>>,
      std::enable_if_t<std::is_same_v<typename T::size_type, decltype(T::max_size())>>,
      std::enable_if_t<
        std::is_same_v<typename T::size_type, decltype(std::declval<T>().max_alloc())>>,
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