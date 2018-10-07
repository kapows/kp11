#pragma once

#include "traits.h" // is_resource_v

#include <tuple> // tuple, get
#include <utility> // piecewise_construct, index_sequence, index_sequence_for

namespace kp11
{
  /// Allocates from `Secondary` if allocation from `Primary` returns `nullptr`.
  ///
  /// @tparam Primary Meets the `Owner` concept
  /// @tparam Secondary Meets the `Resource` concept
  template<typename Primary, typename Secondary>
  class fallback
  {
    static_assert(is_resource_v<Primary>);
    static_assert(is_resource_v<Secondary>);

  public: // typedefs
    /// Pointer type
    using pointer = typename Primary::pointer;
    /// Size type
    using size_type = typename Primary::size_type;

  public: // constructors
    /// Defaulted because of the forwarding constructor.
    fallback() = default;
    /// Fowarding constructor for `Primary` and `Secondary`.
    ///
    /// @param first_args Constructor arguments to `Primary`.
    /// @param second_args Constructor arguments to `Secondary`.
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
    /// Calls `Primary::allocate`, if that fails calls `Secondary::allocate`.
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    /// @returns (failure) `nullptr`.
    ///
    /// @post (success) (Return value) will not be returned again until it has been `deallocated`.
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (auto ptr = primary.allocate(bytes, alignment))
      {
        return ptr;
      }
      return secondary.allocate(bytes, alignment);
    }
    /// If `ptr` is owned by `Primary` then calls `Primary:deallocate` else calls
    /// `Secondary::deallocate`. If `Primary` supplies a `deallocate` that returns a value
    /// convertible to `bool` then that function will be used to determine if `Primary` owns `ptr`.
    ///
    /// @param ptr Pointer to the beginning of memory returned by a call to `allocate`.
    /// @param bytes Corresposing argument to call to `allocate`.
    /// @param alignment Corresposing argument to call to `allocate`.
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      // It may be trivial for a type to return success or failure in it's deallocate function, if
      // if is then it should do so.
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
      // If it is not trivial then we can still determine ownership through operator[].
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
    /// @returns Reference to `Primary`.
    Primary & get_primary() noexcept
    {
      return primary;
    }
    /// @returns Reference to `Primary`.
    Primary const & get_primary() const noexcept
    {
      return primary;
    }
    /// @returns Reference to `Secondary`.
    Secondary & get_secondary() noexcept
    {
      return secondary;
    }
    /// @returns Reference to `Secondary`.
    Secondary const & get_secondary() const noexcept
    {
      return secondary;
    }

  private: // variables
    Primary primary;
    Secondary secondary;
  };
}