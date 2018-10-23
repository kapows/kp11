#pragma once

#include <cassert> // assert
#include <cstddef> // size_t

namespace kp11
{
  /// Only allocates `nullptr`
  ///
  /// @tparam Pointer Pointer type
  /// @tparam SizeType Size type
  template<typename Pointer, typename SizeType>
  class basic_nullocator
  {
  public: // typedefs
    /// Pointer type
    using pointer = Pointer;
    /// Size type
    using size_type = SizeType;

  public:
    /// @returns `nullptr`
    pointer allocate(size_type, size_type) noexcept
    {
      return nullptr;
    }
    /// @pre `ptr == nullptr`
    void deallocate([[maybe_unused]] pointer ptr, size_type, size_type) noexcept
    {
      assert(ptr == nullptr);
    }
  };
  /// Typedef of basic_nullocator with `void *` as the `pointer` and `std::size_t` as the
  /// `size_type`.
  using nullocator = basic_nullocator<void *, std::size_t>;
}