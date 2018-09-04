#pragma once

#include "traits.h" // is_strategy_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less
#include <map> // map
#include <memory> // pointer_traits
#include <utility> // forward_as_tuple

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
      bool operator<(mem_block const & rhs) const noexcept
      {
        return std::less<pointer>()(first, rhs.first) && std::less<pointer>()(last, rhs.last);
      }
    };

  }
  template<std::size_t Bytes,
    std::size_t Alignment,
    typename Strategy,
    typename Resource,
    std::size_t Size = 0>
  class cascade
  {
    static_assert(is_strategy_v<Strategy>, "cascade requires Strategy to be a Strategy");
    static_assert(is_resource_v<Resource>, "cascade requires Resource to be a Resource");

  public: // typedefs
    using pointer = typename Resource::pointer;
    using size_type = typename Resource::size_type;
    using mem_block = cascade_detail::mem_block<pointer, size_type>;

  private: // typedefs
    using strategies_type = std::map<mem_block, Strategy>;

  public: // typedefs
    using iterator = typename strategies_type::iterator;
    using const_iterator = typename strategies_type::const_iterator;

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      for (auto && s : strategies)
      {
        if (auto ptr = s.second.allocate(bytes, alignment); ptr != nullptr)
        {
          return ptr;
        }
      }
      // can't allocate from current strategies, probably out of space, try to allocate a new one
      if (auto ptr = resource.allocate(Bytes, Alignment); ptr != nullptr)
      {
        try
        {
          auto [it, b] = strategies.emplace(std::piecewise_construct,
            std::forward_as_tuple(ptr, Bytes),
            std::forward_as_tuple(ptr, Bytes, Alignment));
          return it->second.allocate(bytes, alignment);
        }
        catch (...)
        {
          resource.deallocate(ptr, Bytes, Alignment);
          throw;
        }
      }
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      assert(strategies.find(ptr) != strategies.end());
      strategies[ptr].deallocate(ptr, bytes, alignment);
    }

  public: // iterators
    iterator begin() noexcept
    {
      return strategies.begin();
    }
    iterator end() noexcept
    {
      return strategies.end();
    }
    const_iterator begin() const noexcept
    {
      return strategies.begin();
    }
    const_iterator end() const noexcept
    {
      return strategies.end();
    }
    const_iterator cbegin() const noexcept
    {
      return strategies.cbegin();
    }
    const_iterator cend() const noexcept
    {
      return strategies.cend();
    }

  public: // observers
    iterator find(pointer ptr) noexcept
    {
      return strategies.find(ptr);
    }
    const_iterator find(pointer ptr) const noexcept
    {
      return strategies.find(ptr);
    }

  private: // variables
    strategies_type strategies;
    Resource resource;
  };
}
