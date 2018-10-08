#pragma once

#include "traits.h" // is_resource_v

#include <cstddef> // size_t
#include <tuple> // tuple, get
#include <utility> // index_sequence, index_sequence_for

namespace kp11
{
  /// @brief Sizes less than `threshold` will be allocated by `Small`, greater or equal will be
  /// allocated by `Large`.
  ///
  /// @tparam Small Meets the `Resource` concept
  /// @tparam Large Meets the `Resource` concept
  template<typename Small, typename Large>
  class segregator
  {
    static_assert(is_resource_v<Small>);
    static_assert(is_resource_v<Large>);

  public: // typedefs
    /// Pointer type
    using pointer = typename Small::pointer;
    /// Size type
    using size_type = typename Small::size_type;

  public: // constructor
    /// @param threshold Threshold size in bytes.
    segregator(size_type threshold) noexcept : threshold(threshold)
    {
    }
    /// Fowarding constructor for `Small` and `Large`.
    ///
    /// @param threshold Threshold size in bytes.
    /// @param first_args Constructor arguments to `Small`.
    /// @param second_args Constructor arguments to `Large`.
    template<typename... Args1, typename... Args2>
    segregator(size_type threshold,
      std::tuple<Args1...> first_args,
      std::tuple<Args2...> second_args) noexcept :
        segregator(threshold,
          first_args,
          second_args,
          std::index_sequence_for<Args1...>(),
          std::index_sequence_for<Args2...>())
    {
    }

  private: // constructor helper
    /// Constructor that unpacks `tuple` arguments.
    template<std::size_t... Is1, typename... Args1, std::size_t... Is2, typename... Args2>
    segregator(size_type threshold,
      std::tuple<Args1...> & first_args,
      std::tuple<Args2...> & second_args,
      std::index_sequence<Is1...>,
      std::index_sequence<Is2...>) noexcept :
        threshold(threshold),
        small(std::forward<Args1>(std::get<Is1>(first_args))...),
        large(std::forward<Args2>(std::get<Is2>(second_args))...)
    {
    }

  public: // modifier
    /// If `bytes < threshold` calls `Small::allocate` else calls `Large::allocate`.
    ///
    /// @param bytes Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a memory block of size `bytes` aligned to
    /// `alignment`.
    /// @returns (failure) `nullptr`.
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (bytes < threshold)
      {
        return small.allocate(bytes, alignment);
      }
      else
      {
        return large.allocate(bytes, alignment);
      }
    }
    /// If `bytes < threshold` calls `Small::deallocate` else calls `Large::deallocate`.
    ///
    /// @param ptr Pointer to the beginning of memory returned by a call to `allocate`.
    /// @param bytes Corresposing argument to call to `allocate`.
    /// @param alignment Corresposing argument to call to `allocate`.
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (bytes < threshold)
      {
        small.deallocate(ptr, bytes, alignment);
      }
      else
      {
        large.deallocate(ptr, bytes, alignment);
      }
    }

  public: // accessors
    /// @returns Reference to `Small`.
    Small & get_small() noexcept
    {
      return small;
    }
    /// @returns Reference to `Small`.
    Small const & get_small() const noexcept
    {
      return small;
    }
    /// @returns Reference to `Large`.
    Large & get_large() noexcept
    {
      return large;
    }
    /// @returns Reference to `Large`.
    Large const & get_large() const noexcept
    {
      return large;
    }

  private: // variables
    size_type threshold;
    Small small;
    Large large;
  };
}