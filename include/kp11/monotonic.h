#pragma once

#include "traits.h" // is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less, less_equal
#include <memory> // pointer_traits
#include <utility> // forward, exchange

namespace kp11
{
  /**
   * @brief Allocates memory by incrementing a pointer.
   *
   * @tparam Replicas number of times to replicate
   * @tparam Upstream type that meets the `Resource` concept. This is where memory will be allocated
   * from.
   */
  template<std::size_t Replicas, typename Upstream>
  class monotonic
  {
    static_assert(is_resource_v<Upstream>);

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
     * @brief Construct a new monotonic object
     *
     * @param bytes Size in bytes of memory to alloate from `Upstream` per replica
     * @param alignment Size in bytes of memory to alloate from `Upstream` per replica
     * @param args `Upstream` constructor arguments
     */
    template<typename... Args>
    monotonic(size_type bytes, size_type alignment, Args &&... args) noexcept :
        bytes(bytes), alignment(alignment), upstream(std::forward<Args>(args)...)
    {
    }
    /**
     * @brief Delete copy constructor since a resource is being held and managed.
     */
    monotonic(monotonic const &) = delete;
    /**
     * @brief Delete copy assignment since a resource is being held and managed.
     */
    monotonic & operator=(monotonic const &) = delete;
    /**
     * @brief Destroy the monotonic object. Deallocate all memory back to `Upstream`.
     */
    ~monotonic() noexcept
    {
      while (length > 1)
      {
        pop_back();
      }
    }

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     *
     * @par Complexity
     * `O(1)`
     * @pre `alignment` must be at most the one passed into the constructor
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      assert(this->alignment % alignment == 0);
      bytes = round_up_to_our_alignment(bytes);
      if (auto ptr = allocate_from_current_replica(bytes))
      {
        return static_cast<pointer>(ptr);
      }
      else if (push_back())
      {
        // this call should not fail as a full buffer should be able to fulfil any request made
        auto ptr = allocate_from_current_replica(bytes);
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
     * @par Complexity
     * `O(0)`
     *
     * @note No-op
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }

  private: // allocate helpers
    size_type round_up_to_our_alignment(size_type bytes) const noexcept
    {
      return bytes == 0 ? alignment : (bytes / alignment + (bytes % alignment != 0)) * alignment;
    }
    unsigned_char_pointer allocate_from_current_replica(size_type bytes) noexcept
    {
      assert(bytes % alignment == 0);
      if (auto space = static_cast<size_type>(lasts[length - 1] - ptr); bytes <= space)
      {
        return std::exchange(ptr, ptr + bytes);
      }
      return nullptr;
    }

  private: // modifiers
    /**
     * @brief Add a replica to the end of our container. Calls Upstream::allocate.
     * Increases length by 1.
     *
     * @return true if successful
     * @return false otherwise
     */
    bool push_back() noexcept
    {
      if (length != Replicas + 1)
      {
        if (auto ptr = static_cast<unsigned_char_pointer>(upstream.allocate(bytes, alignment)))
        {
          this->ptr = ptr;
          lasts[length++] = ptr + bytes;
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
      assert(length > 1);
      upstream.deallocate(static_cast<pointer>(lasts[length - 1] - bytes), bytes, alignment);
      --length;
      ptr = length ? lasts[length - 1] : nullptr;
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
      for (std::size_t i = 1; i < length; ++i)
      {
        auto const first = lasts[i] - bytes;
        if (std::less_equal<pointer>()(static_cast<pointer>(first), ptr) &&
            std::less<pointer>()(ptr, static_cast<pointer>(lasts[i])))
        {
          return static_cast<pointer>(first);
        }
      }
      return nullptr;
    }

  private: // variables
    std::size_t length = 1; // length starts at 1 with lasts[0] being the bootstrap pointer
    unsigned_char_pointer ptr = nullptr; // pointer used for allocation
    /**
     * @brief pointers to the end of memory (beginning is lasts[i] - Bytes).
     * lasts[0] is a bootstrap pointer so it must exist.
     * bootstrap exists so that we don't have to check to see if we've been initialized.
     * bootstrap is always nullptr
     */
    unsigned_char_pointer lasts[Replicas + 1]{nullptr};
    size_type const bytes;
    size_type const alignment;
    Upstream upstream;
  };
}