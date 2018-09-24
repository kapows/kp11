#pragma once

#include "traits.h" // is_marker_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less, less_equal
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
  class free_block
  {
    static_assert(is_marker_v<Marker>);

  public: // typedefs
    /**
     * @brief pointer type
     */
    using pointer = typename Upstream::pointer;
    /**
     * @brief size type
     */
    using size_type = typename Upstream::size_type;

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
        bytes(bytes), alignment(alignment), upstream(std::forward<Args>(args)...)
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
      while (length)
      {
        pop_back();
      }
    }

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     *
     * @pre `alignment` must be at most the one passed into the constructor
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(this->alignment % alignment == 0);
      auto const num_blocks = size_from(bytes);
      if (auto ptr = allocate_from_current_replicas(num_blocks))
      {
        return static_cast<pointer>(ptr);
      }
      else if (push_back())
      {
        // allocation here should not fail as a full buffer should be able to fulfil any request
        auto ptr = allocate_from_replica(length - 1, num_blocks);
        assert(ptr != nullptr);
        return static_cast<pointer>(ptr);
      }
      else
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
      auto p = static_cast<unsigned_char_pointer>(ptr);
      if (auto i = find(p); i != Replicas)
      {
        markers[i].reset(index_from(ptrs[i], p), size_from(bytes));
        return true;
      }
      return false;
    }

  public: // allocate helper
    unsigned_char_pointer allocate_from_replica(std::size_t index, std::size_t num_blocks) noexcept
    {
      if (auto i = markers[index].set(num_blocks); i != Marker::size())
      {
        return ptrs[index] + i * this->bytes;
      }
      return nullptr;
    }
    unsigned_char_pointer allocate_from_current_replicas(std::size_t num_blocks) noexcept
    {
      for (std::size_t i = 0; i < length; ++i)
      {
        if (auto ptr = allocate_from_replica(i, num_blocks))
        {
          return ptr;
        }
      }
      return nullptr;
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

  public: // accessors
    /**
     * @brief Get the upstream object
     *
     * @return Upstream&
     */
    Upstream & get_upstream() noexcept
    {
      return upstream;
    }
    /**
     * @brief Get the upstream object
     *
     * @return Upstream const&
     */
    Upstream const & get_upstream() const noexcept
    {
      return upstream;
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
      for (std::size_t i = 0; i < length; ++i)
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
     * Increases length by 1.
     *
     * @return true if successful
     * @return false otherwise
     */
    bool push_back() noexcept
    {
      if (length != Replicas)
      {
        if (auto ptr = upstream.allocate(bytes * Marker::size(), alignment))
        {
          ptrs[length] = static_cast<unsigned_char_pointer>(ptr);
          new (&markers[length]) Marker();
          ++length;
          return true;
        }
      }
      return false;
    }
    /**
     * @brief Deallocate memory from the back, back to `Upstream`. Decreases length by 1.
     */
    void pop_back() noexcept
    {
      assert(length > 0);
      upstream.deallocate(static_cast<pointer>(ptrs[length - 1]), bytes, alignment);
      markers[length - 1].~Marker();
      --length;
    }

  private: // Marker helper functions
    typename Marker::size_type size_from(size_type bytes) const noexcept
    {
      // 1 block minimum
      // mod is required to deal with non BlockSize sizes
      size_type s = bytes == 0 ? 1 : bytes / this->bytes + (bytes % this->bytes != 0);
      return static_cast<typename Marker::size_type>(s);
    }
    typename Marker::size_type index_from(
      unsigned_char_pointer first, unsigned_char_pointer ptr) const noexcept
    {
      auto p = (ptr - first) / bytes;
      return static_cast<typename Marker::size_type>(p);
    }

  private: // variables
    std::size_t length = 0;
    unsigned_char_pointer ptrs[Replicas];
    union {
      Marker markers[Replicas];
    };
    size_type const bytes;
    size_type const alignment;
    Upstream upstream;
  };
}