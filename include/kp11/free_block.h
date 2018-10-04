#pragma once

#include "traits.h" // is_marker_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t, byte
#include <functional> // less, less_equal
#include <memory> // pointer_traits
#include <utility> // forward

namespace kp11
{
  /// Allocate memory in blocks instead of per byte.
  /// Allocations and deallocations will defer to `Marker` to determine functionality.
  /// * `Allocations` is the maximum of successful allocation requests to `Upstream` and the number
  /// of `Markers` to hold
  /// * `Marker` meets the `Marker` concept
  /// * `Upstream` meets the `Resource` concept
  template<std::size_t Allocations, typename Marker, typename Upstream>
  class free_block
  {
    static_assert(is_marker_v<Marker>);
    static_assert(is_resource_v<Upstream>);

  public: // typedefs
    using pointer = typename Upstream::pointer;
    using size_type = typename Upstream::size_type;

  private: // typedefs
    using byte_pointer = typename std::pointer_traits<pointer>::template rebind<std::byte>;

  public: // constructors
    /// * `bytes` is the size in bytes of memory to request from `Upstream`
    /// * `alignment` is the alignment in bytes of memory to request from `Upstream`
    /// * `args` are the constructor arguments to `Upstream`
    template<typename... Args>
    free_block(size_type bytes, size_type alignment, Args &&... args) noexcept :
        bytes(bytes), alignment(alignment), upstream(std::forward<Args>(args)...)
    {
    }
    /// Deleted because a resource is being held and managed.
    free_block(free_block const &) = delete;
    /// Deleted because a resource is being held and managed.
    free_block & operator=(free_block const &) = delete;
    /// Defined because we need to release all allocated memory back to `Upstream`.
    ~free_block() noexcept
    {
      release();
    }

  public: // modifiers
    /// * Precondition `alignment (from ctor) % alignment == 0`
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
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      auto p = static_cast<byte_pointer>(ptr);
      if (auto i = find(p); i != Allocations)
      {
        markers[i].reset(index_from(ptrs[i], p), size_from(bytes));
        return true;
      }
      return false;
    }

    void release() noexcept
    {
      while (length)
      {
        pop_back();
      }
    }

  private: // allocate helper
    byte_pointer allocate_from_replica(std::size_t index, std::size_t num_blocks) noexcept
    {
      if (auto i = markers[index].set(num_blocks); i != Marker::size())
      {
        return ptrs[index] + i * this->bytes;
      }
      return nullptr;
    }
    byte_pointer allocate_from_current_replicas(std::size_t num_blocks) noexcept
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
    pointer operator[](pointer ptr) const noexcept
    {
      if (auto i = find(static_cast<byte_pointer>(ptr)); i != Allocations)
      {
        return static_cast<pointer>(ptrs[i]);
      }
      return nullptr;
    }

  public: // accessors
    Upstream & get_upstream() noexcept
    {
      return upstream;
    }
    Upstream const & get_upstream() const noexcept
    {
      return upstream;
    }

  private: // operator[] helper
    /// Finds the index of the memory block to which `ptr` is pointing.
    /// * Returns `Allocations` on failure
    std::size_t find(byte_pointer ptr) const noexcept
    {
      for (std::size_t i = 0; i < length; ++i)
      {
        if (std::less_equal<pointer>()(ptrs[i], ptr) &&
            std::less<pointer>()(ptr, ptrs[i] + bytes * Marker::size()))
        {
          return i;
        }
      }
      return Allocations;
    }

  private: // modifiers
    /// Allocates a new block of memory from `Upstream` and adds another `Marker` for it.
    bool push_back() noexcept
    {
      if (length != Allocations)
      {
        if (auto ptr = upstream.allocate(bytes * Marker::size(), alignment))
        {
          ptrs[length] = static_cast<byte_pointer>(ptr);
          new (&markers[length]) Marker();
          ++length;
          return true;
        }
      }
      return false;
    }
    /// Deallocates allocated memory back to `Upstream` and destroys it's associated `Marker`.
    void pop_back() noexcept
    {
      assert(length > 0);
      upstream.deallocate(static_cast<pointer>(ptrs[length - 1]), bytes, alignment);
      markers[length - 1].~Marker();
      --length;
    }

  private: // Marker helper functions
    /// Convert from `bytes` to number of blocks to use with `Marker`.
    typename Marker::size_type size_from(size_type bytes) const noexcept
    {
      // 1 block minimum
      // moduloc is required to deal with non BlockSize sizes
      size_type s = bytes == 0 ? 1 : bytes / this->bytes + (bytes % this->bytes != 0);
      return static_cast<typename Marker::size_type>(s);
    }
    /// Convert from `byte_pointer` to an index to use with `Marker`
    typename Marker::size_type index_from(byte_pointer first, byte_pointer ptr) const noexcept
    {
      auto p = (ptr - first) / bytes;
      return static_cast<typename Marker::size_type>(p);
    }

  private: // variables
    /// Only modified by `push_back` and `pop_back`.
    std::size_t length = 0;
    /// Holds pointers to memory allocated by `Upstream`.
    byte_pointer ptrs[Allocations];
    /// Is in a union to avoid construction of all `Marker`s at object construction time.
    union {
      /// Holds `Markers` associated with each corresponding replica of `ptrs`.
      Marker markers[Allocations];
    };
    /// Size in bytes of memory to allocate from `Upstream`.
    size_type const bytes;
    /// Size in bytes of alignment of memory to allocate from `Upstream`.
    size_type const alignment;
    Upstream upstream;
  };
}