#pragma once

#include "traits.h" // is_resource_v

#include <array> // array
#include <cassert> // assert
#include <cstddef> // size_t
#include <memory> // pointer_traits, align
#include <utility> // exchange

namespace kp11
{
  /**
   * @brief Allocates memory by incrementing a pointer.
   *
   * @tparam Bytes size of memory in bytes
   * @tparam Alignment alignment of memory in bytes
   * @tparam Replicas number of times to replicate
   * @tparam Upstream type that meets the `Resource` concept. This is where memory will be allocated
   * from.
   */
  template<std::size_t Bytes, std::size_t Alignment, std::size_t Replicas, typename Upstream>
  class monotonic : public Upstream
  {
    static_assert(is_resource_v<Upstream>);

  public: // typedefs
    using typename Upstream::pointer;
    using typename Upstream::size_type;

  private: // typedefs
    using unsigned_char_pointer =
      typename std::pointer_traits<pointer>::template rebind<unsigned char>;

  public: // constructors
    using Upstream::Upstream;
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
      clear();
    }

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     *
     * @par Complexity
     * `O(1)`
     */
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      if (auto ptr = allocate_from_current_replica(bytes, alignment))
      {
        return ptr;
      }
      else if (push_back()) // replica added
      {
        // this call should not fail as a full buffer should be able to fulfil any request made
        auto ptr = allocate_from_current_replica(bytes, alignment);
        assert(ptr != nullptr);
        return ptr;
      }
      else // not enough space in current replica and adding replica failed
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

  private: // allocate helper
    pointer allocate_from_current_replica(size_type bytes, size_type alignment) noexcept
    {
      using std::align;
      auto space = static_cast<size_type>(lasts[length - 1] - ptr);
      auto p = static_cast<pointer>(ptr);
      if (align(alignment, bytes, p, space))
      {
        ptr = static_cast<unsigned_char_pointer>(p) + bytes;
        return p;
      }
      return nullptr;
    }

  private: // modifiers
    bool push_back() noexcept
    {
      // too many replicas
      if (length == Replicas + 1)
      {
        return false;
      }
      else if (auto p = Upstream::allocate(Bytes, Alignment)) // allocate new memory from upstream
      {
        ptr = static_cast<unsigned_char_pointer>(p);
        lasts[length++] = ptr + Bytes;
        return true;
      }
      else // failed to allocate memory from upstream
      {
        return false;
      }
    }
    void clear() noexcept
    {
      while (length > 1)
      {
        Upstream::deallocate(static_cast<pointer>(lasts[length - 1] - Bytes), Bytes, Alignment);
        --length;
      }
    }

  public: // observers
    pointer operator[](pointer ptr) const noexcept
    {
      for (std::size_t i = 1; i < length; ++i)
      {
        auto const first = lasts[i] - Bytes;
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
     */
    std::array<unsigned_char_pointer, Replicas + 1> lasts{nullptr};
  };
}