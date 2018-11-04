#pragma once

#include "traits.h" // is_resource_v, resource_traits

#include <cassert> // assert
#include <cstddef> // size_t

namespace kp11
{
  /// @brief Sizes less than or equal to `Threshold` will be allocated by `Small`, greater will be
  /// allocated by `Large`.
  ///
  /// @tparam Threshold Threshold size in bytes.
  /// @tparam Small Meets the `Resource` concept
  /// @tparam Large Meets the `Resource` concept
  template<std::size_t Threshold, typename Small, typename Large>
  class segregator
  {
    static_assert(is_resource_v<Small>);
    static_assert(is_resource_v<Large>);

  public: // typedefs
    /// Pointer type
    using pointer = typename Small::pointer;
    /// Size type
    using size_type = typename resource_traits<Small>::size_type;

  public: // constants
    /// Threshold size in bytes.
    static constexpr auto threshold = Threshold;

  public: // capacity
    /// @returns The maximum allocation size supported. This is `Large::max_size()`.
    static constexpr size_type max_size() noexcept
    {
      return resource_traits<Large>::max_size();
    }

  public: // modifier
    /// If `size <= threshold` calls `Small::allocate` else calls `Large::allocate`.
    ///
    /// @param size Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @returns (success) Pointer to the beginning of a suitable memory block.
    /// @returns (failure) `nullptr`.
    ///
    /// @pre `size <= max_size()`
    pointer allocate(size_type size, size_type alignment) noexcept
    {
      assert(size <= max_size());
      if (size <= threshold)
      {
        return small.allocate(size, alignment);
      }
      else
      {
        return large.allocate(size, alignment);
      }
    }
    /// If `size <= threshold` calls `Small::deallocate` else calls `Large::deallocate`.
    ///
    /// @param ptr Pointer to the beginning of memory returned by a call to `allocate`.
    /// @param size Corresposing argument to call to `allocate`.
    /// @param alignment Corresposing argument to call to `allocate`.
    void deallocate(pointer ptr, size_type size, size_type alignment) noexcept
    {
      if (size <= threshold)
      {
        small.deallocate(ptr, size, alignment);
      }
      else
      {
        large.deallocate(ptr, size, alignment);
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
    Small small;
    Large large;
  };
}