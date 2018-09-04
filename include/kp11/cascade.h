#pragma once

#include "traits.h" // is_strategy_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less
#include <memory> // pointer_traits
#include <utility> // forward_as_tuple
#include <vector> // vector

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
      friend bool operator<(pointer lhs, mem_block const & rhs) noexcept
      {
        return std::less<pointer>()(lhs, rhs.first) && std::less<pointer>()(lhs, rhs.last);
      }
      friend bool operator<(mem_block const & lhs, pointer rhs) noexcept
      {
        return std::less<pointer>()(lhs.first, rhs) && std::less<pointer>()(lhs.last, rhs);
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
    using strategies_type = std::vector<Strategy>;
    using mem_blocks_type = std::vector<mem_block>;
    class const_iterator;
    class iterator
    {
      friend class cascade;
      friend class const_iterator;

    private: // typedefs
      using mem_blocks_const_iterator = typename mem_blocks_type::const_iterator;
      using strategies_iterator = typename strategies_type::iterator;

    public: // typedefs
      using iterator_category = std::random_access_iterator_tag;
      using reference = std::pair<mem_block const &, Strategy &>;
      using value_type = reference;
      using pointer = value_type *;
      using difference_type = typename mem_blocks_const_iterator::difference_type;

    public: // constructors
      iterator() = default;

    private: // constructors
      iterator(mem_blocks_const_iterator mi, strategies_iterator si) : mi(mi), si(si)
      {
      }

    public: // accessors
      reference operator*() noexcept
      {
        return std::forward_as_tuple(*mi, *si);
      }
      reference operator[](difference_type i) noexcept
      {
        return std::forward_as_tuple(mi[i], si[i]);
      }

    public: // modifiers
      iterator & operator++() noexcept
      {
        ++mi;
        ++si;
        return *this;
      }
      iterator operator++(int) noexcept
      {
        auto c = *this;
        ++this;
        return c;
      }
      iterator & operator--() noexcept
      {
        --mi;
        --si;
        return *this;
      }
      iterator operator--(int) noexcept
      {
        auto c = *this;
        --this;
        return c;
      }
      iterator & operator+=(difference_type rhs) noexcept
      {
        mi += rhs;
        si += rhs;
        return *this;
      }
      iterator & operator-=(difference_type rhs) noexcept
      {
        mi -= rhs;
        si -= rhs;
        return *this;
      }
      friend iterator operator+(iterator const & lhs, difference_type rhs) noexcept
      {
        iterator c = lhs;
        c += rhs;
        return c;
      }
      friend iterator operator+(difference_type lhs, iterator const & rhs) noexcept
      {
        iterator c = rhs;
        c += lhs;
        return c;
      }

      friend iterator operator-(iterator const & lhs, difference_type rhs) noexcept
      {
        iterator c = lhs;
        c -= rhs;
        return c;
      }

    public: // comparison
      friend bool operator<(iterator const & lhs, iterator const & rhs) noexcept
      {
        return lhs.mi < rhs.mi;
      }
      friend bool operator<=(iterator const & lhs, iterator const & rhs) noexcept
      {
        return lhs.mi <= rhs.mi;
      }
      friend bool operator>(iterator const & lhs, iterator const & rhs) noexcept
      {
        return lhs.mi > rhs.mi;
      }
      friend bool operator>=(iterator const & lhs, iterator const & rhs) noexcept
      {
        return lhs.mi >= rhs.mi;
      }
      friend bool operator==(iterator const & lhs, iterator const & rhs) noexcept
      {
        return lhs.mi == rhs.mi;
      }
      friend bool operator!=(iterator const & lhs, iterator const & rhs) noexcept
      {
        return lhs.mi != rhs.mi;
      }

    private: // variables
      mem_blocks_const_iterator mi;
      strategies_iterator si;
    };
    class const_iterator
    {
      friend class cascade;

    private: // typedefs
      using mem_blocks_const_iterator = typename mem_blocks_type::const_iterator;
      using strategies_const_iterator = typename strategies_type::const_iterator;

    public: // typedefs
      using iterator_category = std::random_access_iterator_tag;
      using reference = std::pair<mem_block const &, Strategy const &>;
      using value_type = reference;
      using pointer = value_type *;
      using difference_type = typename mem_blocks_const_iterator::difference_type;

    public: // constructors
      const_iterator() = default;
      const_iterator(iterator const & rhs) : mi(rhs.mi), si(rhs.si)
      {
      }

    private: // constructors
      const_iterator(mem_blocks_const_iterator mi, strategies_const_iterator si) : mi(mi), si(si)
      {
      }

    public: // accessors
      reference operator*() const noexcept
      {
        return std::forward_as_tuple(*mi, *si);
      }
      reference operator[](difference_type i) const noexcept
      {
        return std::forward_as_tuple(mi[i], si[i]);
      }

    public: // modifiers
      const_iterator & operator++() noexcept
      {
        ++mi;
        ++si;
        return *this;
      }
      const_iterator operator++(int) noexcept
      {
        auto c = *this;
        ++this;
        return c;
      }
      const_iterator & operator--() noexcept
      {
        --mi;
        --si;
        return *this;
      }
      const_iterator operator--(int) noexcept
      {
        auto c = *this;
        --this;
        return c;
      }
      const_iterator & operator+=(difference_type rhs) noexcept
      {
        mi += rhs;
        si += rhs;
        return *this;
      }
      const_iterator & operator-=(difference_type rhs) noexcept
      {
        mi -= rhs;
        si -= rhs;
        return *this;
      }
      friend const_iterator operator+(const_iterator const & lhs, difference_type rhs) noexcept
      {
        const_iterator c = lhs;
        c += rhs;
        return c;
      }
      friend const_iterator operator+(difference_type lhs, const_iterator const & rhs) noexcept
      {
        const_iterator c = rhs;
        c += lhs;
        return c;
      }

      friend const_iterator operator-(const_iterator const & lhs, difference_type rhs) noexcept
      {
        const_iterator c = lhs;
        c -= rhs;
        return c;
      }

    public: // comparison
      friend bool operator<(const_iterator const & lhs, const_iterator const & rhs) noexcept
      {
        return lhs.mi < rhs.mi;
      }
      friend bool operator<=(const_iterator const & lhs, const_iterator const & rhs) noexcept
      {
        return lhs.mi <= rhs.mi;
      }
      friend bool operator>(const_iterator const & lhs, const_iterator const & rhs) noexcept
      {
        return lhs.mi > rhs.mi;
      }
      friend bool operator>=(const_iterator const & lhs, const_iterator const & rhs) noexcept
      {
        return lhs.mi >= rhs.mi;
      }
      friend bool operator==(const_iterator const & lhs, const_iterator const & rhs) noexcept
      {
        return lhs.mi == rhs.mi;
      }
      friend bool operator!=(const_iterator const & lhs, const_iterator const & rhs) noexcept
      {
        return lhs.mi != rhs.mi;
      }

    private: // variables
      mem_blocks_const_iterator mi;
      strategies_const_iterator si;
    };

  public: // constructors
    cascade()
    {
      mem_blocks.reserve(20);
      strategies.reserve(20);
    }

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      for (auto && s : strategies)
      {
        if (auto ptr = s.allocate(bytes, alignment); ptr != nullptr)
        {
          return ptr;
        }
      }
      // can't allocate from current strategies, probably out of space, try to allocate a new one
      if (mem_blocks.size() != mem_blocks.capacity())
      {
        if (auto ptr = resource.allocate(Bytes, Alignment); ptr != nullptr)
        {
          mem_blocks.emplace_back(ptr, Bytes);
          strategies.emplace_back(ptr, Bytes, Alignment);
          return strategies.back().allocate(bytes, alignment);
        }
      }
      return nullptr;
    }
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      auto it = std::find_if(mem_blocks.begin(), mem_blocks.end(), [ptr](auto const & mem_block) {
        return !(ptr < mem_block) && !(mem_block < ptr);
      });
      assert(it != mem_blocks.end());
      auto i = it - mem_blocks.begin();
      strategies[i].deallocate(ptr, bytes, alignment);
    }

  public: // iterators
    iterator begin() noexcept
    {
      return {mem_blocks.begin(), strategies.begin()};
    }
    iterator end() noexcept
    {
      return {mem_blocks.end(), strategies.end()};
    }
    const_iterator begin() const noexcept
    {
      return {mem_blocks.begin(), strategies.begin()};
    }
    const_iterator end() const noexcept
    {
      return {mem_blocks.end(), strategies.end()};
    }
    const_iterator cbegin() const noexcept
    {
      return {mem_blocks.cbegin(), strategies.cbegin()};
    }
    const_iterator cend() const noexcept
    {
      return {mem_blocks.cend(), strategies.cend()};
    }

  public: // observers
    iterator find(pointer ptr) noexcept
    {
      auto it = std::find_if(mem_blocks.begin(), mem_blocks.end(), [ptr](auto const & mem_block) {
        return !(ptr < mem_block) && !(mem_block < ptr);
      });
      return {it, strategies.begin() + (it - mem_blocks.begin())};
    }
    const_iterator find(pointer ptr) const noexcept
    {
      return {const_cast<cascade *>(this)->find(ptr)};
    }

  private: // variables
    std::vector<mem_block> mem_blocks;
    std::vector<Strategy> strategies;
    Resource resource;
  };
}
