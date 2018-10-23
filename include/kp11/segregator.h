#pragma once

#include "traits.h" // is_resource_v

#include <cstddef> // size_t

namespace kp11
{
  /// @brief Sizes less than `Threshold` will be allocated by `Small`, greater or equal will be
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
    using size_type = typename Small::size_type;

  public: // constants
    /// Threshold size in bytes.
    static constexpr auto threshold = Threshold;

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
    Small small;
    Large large;
  };
}