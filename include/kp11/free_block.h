#pragma once

#include "traits.h" // is_marker_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <utility> // forward

namespace kp11
{
  /**
   * @brief Allocate memory in chunks instead of per byte. Allocations and deallocations will defer
   * to `Marker` to determine functionality.
   *
   * @tparam Replicas number of times to replicate
   * @tparam Marker type that meets the `Marker` concept
   * @tparam Upstream type that meets the `Resource` concept. This is where memory will be allocated
   * from.
   */
  template<std::size_t Replicas, typename Marker, typename Upstream>
  class free_block : public Upstream
  {
    static_assert(is_marker_v<Marker>, "basic_free_block requires Marker to be a Marker");

  public: // typedefs
    using typename Upstream::pointer;
    using typename Upstream::size_type;

  private: // typedefs
    using unsigned_char_pointer =
      typename std::pointer_traits<pointer>::template rebind<unsigned char>;

  public: // constructors
    /**
     * @brief Construct a new free_block object
     *
     * @param bytes Size in bytes of memory to alloate from `Upstream` per replica
     * @param alignment Size in bytes of memory to alloate from `Upstream` per replica
     * @param args `Upstream` constructor arguments
     */
    template<typename... Args>
    free_block(size_type bytes, size_type alignment, Args &&... args) :
        Upstream(std::forward<Args>(args)...), bytes(bytes), alignment(alignment)
    {
    }
    /**
     * @brief Delete copy constructor since a resource is being held and managed.
     */
    free_block(free_block const &) = delete;
    /**
     * @brief Delete copy assignment since a resource is being held and managed.
     */
    free_block & operator=(free_block const &) = delete;
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
      auto const num_blocks = size_from(bytes);
      auto allocate_from_current_markers = [&]() -> pointer {
        for (std::size_t marker_index = 0; marker_index < length; ++marker_index)
        {
          if (auto i = markers[marker_index].set(num_blocks); i != Marker::size())
          {
            return static_cast<pointer>(ptrs[marker_index] + i * this->bytes);
          }
        }
        return nullptr;
      };

      if (auto ptr = allocate_from_current_markers())
      {
        return ptr;
      }
      else if (push_back()) // not enough room
      {
        auto const marker_index = length - 1;
        // this call should not fail as a full buffer should be able to fulfil any request made
        auto i = markers[marker_index].set(num_blocks);
        assert(i != Marker::size());
        return static_cast<pointer>(ptrs[marker_index] + i * this->bytes);
      }
      else // cant push back
      {
        return nullptr;
      }
    }
    /**
     * @copydoc Resource::deallocate
     *
     * @returns true if `ptr` pointers into memory allocated from this resource
     * @returns false otherwise
     */
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (auto i = find(static_cast<unsigned_char_pointer>(ptr)); i != Replicas)
      {
        markers[i].reset(
          index_from(ptrs[i], static_cast<unsigned_char_pointer>(ptr)), size_from(bytes));
        return true;
      }
      return false;
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
      if (auto i = find(static_cast<unsigned_char_pointer>(ptr)); i != Replicas)
      {
        return static_cast<pointer>(ptrs[i]);
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
    std::size_t find(unsigned_char_pointer ptr) const noexcept
    {
      std::size_t i = 0;
      for (; i < length; ++i)
      {
        if (std::less_equal<pointer>()(ptrs[i], ptr) &&
            std::less<pointer>()(ptr, ptrs[i] + bytes * Marker::size()))
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
     * @return true if successful
     * @return false otherwise
     */
    bool push_back() noexcept
    {
      if (length != Replicas)
      {
        if (auto ptr = Upstream::allocate(bytes * Marker::size(), alignment))
        {
          ptrs[length++] = static_cast<unsigned_char_pointer>(ptr);
          return true;
        }
      }
      return false;
    }
    /**
     * @brief Deallocate all memory back to `Upstream`.
     */
    void clear() noexcept
    {
      while (length)
      {
        Upstream::deallocate(static_cast<pointer>(ptrs[length - 1]), bytes, alignment);
        --length;
      }
    }

  private: // Marker helper functions
    typename Marker::size_type size_from(size_type bytes) const noexcept
    {
      // zero bytes is required as well
      if (bytes == 0)
      {
        return static_cast<typename Marker::size_type>(1);
      }
      // mod is required to deal with non BlockSize sizes
      return static_cast<typename Marker::size_type>(
        bytes / this->bytes + (bytes % this->bytes != 0));
    }
    typename Marker::size_type index_from(
      unsigned_char_pointer first, unsigned_char_pointer ptr) const noexcept
    {
      return static_cast<typename Marker::size_type>((ptr - first) / bytes);
    }

  private: // variables
    std::size_t length = 0;
    unsigned_char_pointer ptrs[Replicas];
    Marker markers[Replicas];
    size_type const bytes;
    size_type const alignment;
  };
}