#pragma once

#include "traits.h" // is_strategy_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less, less_equal
#include <memory> // pointer_traits, aligned_storage_t

namespace kp11
{
  namespace cascade_detail
  {
    template<typename pointer, typename size_type>
    pointer advance(pointer ptr, size_type bytes) noexcept
    {
      using char_ptr = typename std::pointer_traits<pointer>::template rebind<char>;
      return static_cast<pointer>(static_cast<char_ptr>(ptr) + bytes);
    }
    template<typename Pointer, typename SizeType>
    class mem_block
    {
    public: // typedefs
      using pointer = Pointer;
      using size_type = SizeType;

    public: // variables
      pointer first;
      pointer last;

    public: // constructors
      mem_block(pointer first, pointer last) noexcept : first(first), last(last)
      {
      }
      mem_block(pointer ptr) noexcept : mem_block(ptr, ptr)
      {
      }
      mem_block(pointer ptr, size_type bytes) noexcept : mem_block(ptr, advance(ptr, bytes))
      {
      }
      bool operator==(mem_block const & rhs) const noexcept
      {
        return first == rhs.first && last == rhs.last;
      }
      bool operator<(mem_block const & rhs) const noexcept
      {
        return std::less<pointer>()(first, rhs.first) && std::less<pointer>()(last, rhs.last);
      }
      bool contains(pointer ptr) const noexcept
      {
        return std::less_equal<pointer>()(first, ptr) && std::less<pointer>()(ptr, last);
      }
    };
  }

  template<std::size_t Bytes,
    std::size_t Alignment,
    std::size_t Size,
    typename Strategy,
    typename Resource>
  class cascade
  {
    static_assert(is_strategy_v<Strategy>, "cascade requires Strategy to be a Strategy");
    static_assert(is_resource_v<Resource>, "cascade requires Resource to be a Resource");

  public: // typedefs
    using pointer = typename Resource::pointer;
    using size_type = typename Resource::size_type;
    using mem_block = cascade_detail::mem_block<pointer, size_type>;

  public: // constructors
    cascade() = default;
    cascade(cascade const &) = delete;
    cascade & operator=(cascade const &) = delete;
    ~cascade() noexcept
    {
      clear();
    }

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      for (std::size_t i = 0; i < length; ++i)
      {
        if (auto ptr = strategies()[i].allocate(bytes, alignment))
        {
          return ptr;
        }
      }
      // can't allocate from current strategies, probably out of space, try to allocate a new one
      if (auto strategy = emplace_back())
      {
        return strategy->allocate(bytes, alignment);
      }
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      (*this)[ptr].second.deallocate(ptr, bytes, alignment);
    }

  public: // observers
    std::pair<mem_block const &, Strategy &> operator[](pointer ptr) noexcept
    {
      auto i = find(ptr);
      assert(i != length);
      return {mem_blocks()[i], strategies()[i]};
    }
    std::pair<mem_block const &, Strategy const &> operator[](pointer ptr) const noexcept
    {
      auto i = find(ptr);
      assert(i != length);
      return {mem_blocks()[i], strategies()[i]};
    }

  private: // operator[] helper
    std::size_t find(pointer ptr) const noexcept
    {
      std::size_t i = 0;
      for (; i < length; ++i)
      {
        if (mem_blocks()[i].contains(ptr))
        {
          break;
        }
      }
      return i;
    }

  private: // modifiers
    Strategy * emplace_back() noexcept
    {
      if (length != Size)
      {
        if (auto ptr = resource.allocate(Bytes, Alignment); ptr != nullptr)
        {
          new (&mem_blocks()[length]) mem_block(ptr, Bytes);
          new (&strategies()[length]) Strategy(ptr, Bytes, Alignment);
          ++length;
          return &strategies()[length - 1];
        }
      }
      return nullptr;
    }
    void clear() noexcept
    {
      while (length)
      {
        resource.deallocate(mem_blocks()[length - 1].first, Bytes, Alignment);
        mem_blocks()[length - 1].~mem_block();
        strategies()[length - 1].~Strategy();
        --length;
      }
    }

  private: // accessors
    mem_block * mem_blocks() noexcept
    {
      return reinterpret_cast<mem_block *>(&mem_block_storage);
    }
    mem_block const * mem_blocks() const noexcept
    {
      return reinterpret_cast<mem_block const *>(&mem_block_storage);
    }
    Strategy * strategies() noexcept
    {
      return reinterpret_cast<Strategy *>(&strategy_storage);
    }
    Strategy const * strategies() const noexcept
    {
      return reinterpret_cast<Strategy const *>(&strategy_storage);
    }

  private: // variables
    std::size_t length = 0;
    std::aligned_storage_t<sizeof(Strategy) * Size, alignof(Strategy)> strategy_storage;
    std::aligned_storage_t<sizeof(mem_block) * Size, alignof(mem_block)> mem_block_storage;
    Resource resource;
  };
}
