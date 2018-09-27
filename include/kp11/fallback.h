#pragma once

#include "traits.h" // is_resource_v

#include <tuple> // tuple, get
#include <utility> // piecewise_construct, index_sequence, index_sequence_for

namespace kp11
{
  /// If allocation from `Primary` is unsuccessful then allocates from `Secondary`.
  /// * `Primary` meets the `Resource` concept
  /// Must either return a convertible bool value on dellocate or return a convertible bool value
  /// from operator[](pointer ptr) in order to determine ownership.
  /// * `Secondary` meets the `Resource` concept
  template<typename Primary, typename Secondary>
  class fallback
  {
    static_assert(is_resource_v<Primary>);
    static_assert(is_resource_v<Secondary>);

  public: // typedefs
    using pointer = typename Primary::pointer;
    using size_type = typename Primary::size_type;

  public: // constructors
    fallback() = default;
    /// * `first_args` are constructor arguments to `Primary`
    /// * `second_args` are constructor arguments to `Secondary`
    template<typename... Args1, typename... Args2>
    fallback(std::piecewise_construct_t,
      std::tuple<Args1...> first_args,
      std::tuple<Args2...> second_args) noexcept :
        fallback(first_args,
          second_args,
          std::index_sequence_for<Args1...>(),
          std::index_sequence_for<Args2...>())
    {
    }

  private: // constructor helper
    /// Constructor that unpacks `tuple` arguments.
    template<std::size_t... Is1, typename... Args1, std::size_t... Is2, typename... Args2>
    fallback(std::tuple<Args1...> & first_args,
      std::tuple<Args2...> & second_args,
      std::index_sequence<Is1...>,
      std::index_sequence<Is2...>) noexcept :
        primary(std::forward<Args1>(std::get<Is1>(first_args))...),
        secondary(std::forward<Args2>(std::get<Is2>(second_args))...)
    {
    }

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (auto ptr = primary.allocate(bytes, alignment))
      {
        return ptr;
      }
      return secondary.allocate(bytes, alignment);
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      // it may be trivial for a type to return success or failure in it's deallocate function, if
      // if is then it should do so
      if constexpr (std::is_convertible_v<bool,
                      decltype(std::declval<Primary>().deallocate(std::declval<pointer>(),
                        std::declval<size_type>(),
                        std::declval<size_type>()))>)
      {
        if (!primary.deallocate(ptr, bytes, alignment))
        {
          secondary.deallocate(ptr, bytes, alignment);
        }
      }
      // if it is not trivial then perhaps we can still determine ownership through operator[]
      // it is an error to use types that do not expose operator[] or return a convertible bool on
      // deallocate as Primary
      else
      {
        if (primary[ptr])
        {
          primary.deallocate(ptr, bytes, alignment);
        }
        else
        {
          secondary.deallocate(ptr, bytes, alignment);
        }
      }
    }

  public: // accessors
    Primary & get_primary() noexcept
    {
      return primary;
    }
    Primary const & get_primary() const noexcept
    {
      return primary;
    }
    Secondary & get_secondary() noexcept
    {
      return secondary;
    }
    Secondary const & get_secondary() const noexcept
    {
      return secondary;
    }

  private: // variables
    Primary primary;
    Secondary secondary;
  };
}