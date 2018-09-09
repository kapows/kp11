#pragma once

#include "traits.h" // is_marker_v
#include "utility.h" // advance

#include <array> // array
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
   * @tparam BlockSize size of memory block in bytes
   * @tparam BlockAlignment alignment of memory block in bytes
   * @tparam Replicas number of times to replicate
   * @tparam Marker type that meets the `Marker` concept
   * @tparam Upstream type that meets the `Resource` concept. This is where memory will be allocated
   * from.
   */
  template<std::size_t BlockSize,
    std::size_t BlockAlignment,
    std::size_t Replicas,
    typename Marker,
    typename Upstream>
  class free_block : public Upstream
  {
    static_assert(is_marker_v<Marker>, "basic_free_block requires Marker to be a Marker");

  public: // typedefs
    using typename Upstream::pointer;
    using typename Upstream::size_type;

  private: // typedefs
    using block_type = std::aligned_storage_t<BlockSize, BlockAlignment>;
    using block_pointer = typename std::pointer_traits<pointer>::template rebind<block_type>;

  public: // constructors
    using Upstream::Upstream;
    /**
     * @brief Destroy the free block object. Deallocate all memory back to `Upstream`.
     */
    ~free_block() noexcept
    {
      clear();
    }

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      auto const num = size_from(bytes);
      // search current markers
      for (std::size_t marker_index = 0; marker_index < length; ++marker_index)
      {
        if (auto i = markers[marker_index].set(num); i != Marker::size())
        {
          return ptrs[marker_index] + i;
        }
      }
      // not enough room in current markers
      if (auto marker_index = push_back(); marker_index != Replicas)
      {
        if (auto i = markers[marker_index].set(num); i != Marker::size())
        {
          return ptrs[marker_index] + i;
        }
      }
      return nullptr;
    }
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      auto i = find(ptr);
      assert(i != Replicas);
      markers[i].reset(index_from(ptrs[i], static_cast<block_pointer>(ptr)), size_from(bytes));
    }

  public: // observers
    /**
     * @brief Check if `ptr` points to memory that was obtained from Upstream.
     *
     * @param ptr pointer to check
     * @returns pointer to the beginning of the memory that was obtained from Upstream
     * @returns nullptr otherwise
     */
    pointer operator[](pointer ptr) const noexcept
    {
      if (auto i = find(ptr); i != Replicas)
      {
        return ptrs[i];
      }
      return nullptr;
    }

  private: // operator[] helper
    /**
     * @brief Return the index of the memory that was obtained from Upstream.
     *
     * @param ptr pointer to find
     * @return the index of the memory that was obtained from Upstream
     * @return `Replicas` otherwise
     */
    std::size_t find(pointer ptr) const noexcept
    {
      std::size_t i = 0;
      for (; i < length; ++i)
      {
        auto first = static_cast<pointer>(ptrs[i]);
        if (std::less_equal<pointer>()(first, ptr) &&
            std::less<pointer>()(ptr, kp11::advance(first, BlockSize * Marker::size())))
        {
          return i;
        }
      }
      return Replicas;
    }

  private: // modifiers
    /**
     * @brief Add a `pointer` and `Marker` to the end of our containers. Calls Upstream::allocate.
     *
     * @return index of the added `pointer` and `Marker`
     * @return `Replicas` if unsuccessful
     */
    std::size_t push_back() noexcept
    {
      if (length != Replicas)
      {
        if (auto ptr = Upstream::allocate(BlockSize * Marker::size(), BlockAlignment);
            ptr != nullptr)
        {
          ptrs[length] = static_cast<block_pointer>(ptr);
          return length++;
        }
      }
      return Replicas;
    }
    /**
     * @brief Deallocate all memory back to `Upstream`.
     */
    void clear() noexcept
    {
      while (length)
      {
        Upstream::deallocate(static_cast<pointer>(ptrs[length - 1]), BlockSize, BlockAlignment);
        --length;
      }
    }

  private: // Marker helper functions
    constexpr typename Marker::size_type size_from(size_type bytes) const noexcept
    {
      return static_cast<typename Marker::size_type>(bytes / BlockSize + (bytes % BlockSize != 0));
    }
    static typename Marker::size_type index_from(block_pointer first, block_pointer ptr) noexcept
    {
      return static_cast<typename Marker::size_type>((ptr - first));
    }

  private: // variables
    std::size_t length = 0;
    std::array<block_pointer, Replicas> ptrs;
    std::array<Marker, Replicas> markers;
  };
}