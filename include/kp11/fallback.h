#pragma once

#include "traits.h" // is_resource_v, is_owner_v

#include <cassert> // assert

namespace kp11
{
  /// @brief Allocate from `Primary`. On failure allocate from `Secondary`.
  ///
  /// @tparam Primary Meets the `Owner` concept.
  /// @tparam Secondary Meets the `Resource` concept.
  template<typename Primary, typename Secondary>
  class fallback
  {
    static_assert(is_owner_v<Primary>);
    static_assert(is_resource_v<Secondary>);

  public: // typedefs
    /// Pointer type
    using pointer = typename Primary::pointer;
    /// Size type
    using size_type = typename Primary::size_type;

  public: // capacity
    /// @returns The maximum allocation size supported.
    static constexpr size_type max_size() noexcept
    {
      return Primary::max_size();
    }

  public: // modifiers
    /// Call `Primary::allocate`. On failure call `Secondary::allocate`.
    ///
    /// @param size Size in bytes of memory to allocate.
    /// @param alignment Alignment of memory to allocate.
    ///
    /// @pre `size <= max_size()`
    ///
    /// @returns (success) Pointer to the beginning of a suitable memory block.
    /// @returns (failure) `nullptr`
    pointer allocate(size_type size, size_type alignment) noexcept
    {
      assert(size <= max_size());
      if (auto ptr = primary.allocate(size, alignment))
      {
        return ptr;
      }
      return secondary.allocate(size, alignment);
    }
    /// If `ptr` is owned by `Primary` then calls `Primary:deallocate` else calls
    /// `Secondary::deallocate`.
    ///
    /// @param ptr Pointer to the beginning of memory returned by a call to `allocate`.
    /// @param size Corresposing argument to call to `allocate`.
    /// @param alignment Corresposing argument to call to `allocate`.
    void deallocate(pointer ptr, size_type size, size_type alignment) noexcept
    {
      if (!owner_traits<Primary>::deallocate(primary, ptr, size, alignment))
      {
        secondary.deallocate(ptr, size, alignment);
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