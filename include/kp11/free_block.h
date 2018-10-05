#pragma once

#include "detail/static_vector.h" // static_vector
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
    /// * `block_size` is the size in bytes of memory blocks. The request to `Upstream` is
    /// `block_size * Marker::size()`.
    /// * `alignment` is the alignment in bytes of memory blocks
    /// * `args` are the constructor arguments to `Upstream`
    template<typename... Args>
    free_block(size_type block_size, size_type alignment, Args &&... args) noexcept :
        block_size(block_size), alignment(alignment), upstream(std::forward<Args>(args)...)
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
      for (std::size_t i = 0, last = ptrs.size(); i < last; ++i)
      {
        if (auto ptr = allocate_from(i, num_blocks))
        {
          return ptr;
        }
      }
      if (push_back())
      {
        // allocation here should not fail as a full buffer should be able to fulfil any request
        auto ptr = allocate_from(ptrs.size() - 1, num_blocks);
        assert(ptr != nullptr);
        return ptr;
      }
      else
      {
        return nullptr;
      }
    }
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      auto p = static_cast<byte_pointer>(ptr);
      if (auto i = find(p); i != ptrs.max_size())
      {
        markers[i].reset(index_from(ptrs[i], p), size_from(bytes));
        return true;
      }
      return false;
    }
    /// Deallocates allocated memory back to `Upstream` and destroys it's associated `Marker`.
    void release() noexcept
    {
      for (auto && p : ptrs)
      {
        upstream.deallocate(static_cast<pointer>(p), request_size(), alignment);
      }
      ptrs.clear();
      markers.clear();
    }

  private: // allocate helper
    pointer allocate_from(std::size_t index, std::size_t num_blocks) noexcept
    {
      if (auto i = markers[index].set(num_blocks); i != Marker::size())
      {
        return static_cast<pointer>(ptrs[index] + static_cast<size_type>(i) * block_size);
      }
      return nullptr;
    }

  public: // observers
    pointer operator[](pointer ptr) const noexcept
    {
      if (auto i = find(static_cast<byte_pointer>(ptr)); i != ptrs.max_size())
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
    /// * Returns `ptrs.max_size()` on failure
    std::size_t find(byte_pointer ptr) const noexcept
    {
      for (std::size_t i = 0, last = ptrs.size(); i < last; ++i)
      {
        if (std::less_equal<pointer>()(ptrs[i], ptr) &&
            std::less<pointer>()(ptr, ptrs[i] + request_size()))
        {
          return i;
        }
      }
      return ptrs.max_size();
    }

  private: // modifiers
    /// Allocates a new block of memory from `Upstream` and adds another `Marker` for it.
    bool push_back() noexcept
    {
      if (ptrs.size() == ptrs.capacity())
      {
        return false;
      }
      if (auto ptr = upstream.allocate(request_size(), alignment))
      {
        ptrs.emplace_back(static_cast<byte_pointer>(ptr));
        markers.emplace_back();
        return true;
      }
      return false;
    }

  private: // size helper
    /// Size in bytes used in request to `Upstream`.
    size_type request_size() const noexcept
    {
      return block_size * static_cast<size_type>(Marker::size());
    }

  private: // Marker helper functions
    /// Convert from `bytes` to number of blocks to use with `Marker`.
    typename Marker::size_type size_from(size_type bytes) const noexcept
    {
      // 1 block minimum
      // moduloc is required to deal with non BlockSize sizes
      size_type s = bytes == 0 ? 1 : bytes / block_size + (bytes % block_size != 0);
      return static_cast<typename Marker::size_type>(s);
    }
    /// Convert from `byte_pointer` to an index to use with `Marker`
    typename Marker::size_type index_from(byte_pointer first, byte_pointer ptr) const noexcept
    {
      auto p = (ptr - first) / block_size;
      return static_cast<typename Marker::size_type>(p);
    }

  private: // variables
    /// Size in bytes of memory blocks.
    size_type const block_size;
    /// Holds pointers to memory allocated by `Upstream`.
    kp11::detail::static_vector<byte_pointer, Allocations> ptrs;
    /// Holds a `Marker` corresponding to each allocation.
    kp11::detail::static_vector<Marker, Allocations> markers;
    /// Size in bytes of alignment of memory blocks.
    size_type const alignment;
    Upstream upstream;
  };
}