#pragma once

#include "traits.h" // is_marker_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <type_traits> // aligned_storage_t

namespace kp11
{
  /**
   * @brief Allocate memory in chucks of `BlockSize` instead of per byte. Memory allocated will
   * always be some multiple of `BlockSize` that is greater than the request. Allocations and
   * deallocations will defer to `Marker` to determine functionality.
   *
   * @tparam Pointer pointer type
   * @tparam SizeType size type
   * @tparam BlockSize size of memory block in bytes
   * @tparam BlockAlignment alignment of memory block in bytes
   * @tparam CascadeSize number of times to replicate
   * @tparam Marker type that meets the `Marker` concept
   * @tparam Upstream type that meets the `Resource` concept. This is where memory will be allocated
   * from.
   */
  template<typename Pointer,
    typename SizeType,
    std::size_t BlockSize,
    std::size_t BlockAlignment,
    std::size_t CascadeSize,
    typename Marker,
    typename Upstream>
  class basic_free_block
  {
    static_assert(is_marker_v<Marker>, "basic_free_block requires Marker to be a Marker");

  public: // typedefs
    /**
     * @brief pointer type
     */
    using pointer = Pointer;
    /**
     * @brief size type
     */
    using size_type = SizeType;

  private: // typedefs
    using block_type = std::aligned_storage_t<BlockSize, 1>;
    using block_pointer = typename std::pointer_traits<pointer>::template rebind<block_type>;

  public: // constructors
    basic_free_block() = default;

  public: // modifiers
    /**
     * @copydoc Strategy::allocate
     *
     * @note Functionality determined by `Marker`.
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      /*
      assert(this->alignment % alignment == 0);
      if (auto i = marker.set(size_from(bytes)); i != marker.size())
      {
        return static_cast<pointer>(&ptr[i]);
      }
      */
      return nullptr;
    }
    /**
     * @copydoc Strategy::deallocate
     *
     * @note Functionality determined by `Marker`.
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      /*
      assert(this->alignment % alignment == 0);
      if (ptr != nullptr)
      {
        marker.reset(index_from(ptr), size_from(bytes));
      }
      */
    }

  public: // observers
    /**
     * @brief Returns a references to the beginning of the original memory block given by
     * Resource::allocate
     *
     * @param ptr pointer that points to within an allocated block
     */
    pointer operator[](pointer ptr) noexcept
    {
      /*
      auto i = find(ptr);
      assert(i != length);
      return {mem_blocks()[i], strategies()[i]};
      */
      return nullptr;
    }
    /**
     * @copydoc cascade::operator[]
     */
    pointer operator[](pointer ptr) const noexcept
    {
      /*
      auto i = find(ptr);
      assert(i != length);
      return {mem_blocks()[i], strategies()[i]};
      */
      return nullptr;
    }

  private: // Marker helper functions
    constexpr typename Marker::size_type size_from(size_type bytes) noexcept
    {
      return static_cast<typename Marker::size_type>(bytes / BlockSize);
    }
    typename Marker::size_type index_from(pointer ptr) noexcept
    {
      return static_cast<typename Marker::size_type>(static_cast<block_pointer>(ptr) - this->ptr);
    }

  private: // variables
    Marker marker;
    block_pointer ptr;
#ifndef NDEBUG
    size_type alignment;
#endif
  };

  /**
   * @brief basic_free_block with `Pointer` as `void *` and `SizeType` as `size_type`
   *
   * @tparam BlockSize size of memory block in bytes
   * @tparam Marker type that fulfils the `Marker` concept
   */
  template<std::size_t BlockSize,
    std::size_t BlockAlignment,
    std::size_t CascadeSize,
    typename Marker,
    typename Upstream>
  using free_block =
    basic_free_block<void *, std::size_t, BlockSize, BlockAlignment, CascadeSize, Marker, Upstream>;
}