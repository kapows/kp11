#pragma once

#include "traits.h" // is_strategy_v, is_resource_v

#include <cassert> // assert
#include <cstddef> // size_t
#include <functional> // less, less_equal
#include <memory> // pointer_traits, aligned_storage_t

namespace kp11
{
  /**
   * @private
   */
  namespace cascade_detail
  {
    /**
     * @brief Advance a `pointer` by `bytes` bytes
     *
     * @tparam pointer pointer type
     * @tparam size_type size type
     * @param ptr origin pointer to advance
     * @param bytes bytes to advance
     * @return pointer advanced pointer
     */
    template<typename pointer, typename size_type>
    pointer advance(pointer ptr, size_type bytes) noexcept
    {
      using char_ptr = typename std::pointer_traits<pointer>::template rebind<char>;
      return static_cast<pointer>(static_cast<char_ptr>(ptr) + bytes);
    }
    /**
     * @brief Structure that stores a pointer to the beginning and a pointer to the end of an
     * allocated block of memory.
     *
     * @tparam Pointer pointer type
     * @tparam SizeType size type
     */
    template<typename Pointer, typename SizeType>
    class mem_block
    {
    public: // typedefs
      /**
       * @brief pointer type
       */
      using pointer = Pointer;
      /**
       * @brief size type
       */
      using size_type = SizeType;

    public: // variables
      /**
       * @brief pointer to the beginning of a memory block
       */
      pointer first;
      /**
       * @brief pointer to the end of a memory block
       */
      pointer last;

    public: // constructors
      /**
       * @brief Construct a new mem block object
       *
       * @param first beginning of memory block
       * @param last end of memory block
       */
      mem_block(pointer first, pointer last) noexcept : first(first), last(last)
      {
      }
      /**
       * @brief Construct a new mem block object
       *
       * @param ptr both the beginning and end of a memory block
       */
      mem_block(pointer ptr) noexcept : mem_block(ptr, ptr)
      {
      }
      /**
       * @brief Construct a new mem block object
       *
       * @param ptr beginning of a memory block
       * @param bytes size of the memory block
       */
      mem_block(pointer ptr, size_type bytes) noexcept : mem_block(ptr, advance(ptr, bytes))
      {
      }
      /**
       * @brief Equality operator
       *
       * @param rhs right hand side
       * @return true all members are equal
       * @return false otherwise
       */
      bool operator==(mem_block const & rhs) const noexcept
      {
        return first == rhs.first && last == rhs.last;
      }
      /**
       * @brief Less than comparison operator
       *
       * @param rhs right hand side
       * @return true if *this < rhs
       * @return false otherwise
       */
      bool operator<(mem_block const & rhs) const noexcept
      {
        return std::less<pointer>()(first, rhs.first) && std::less<pointer>()(last, rhs.last);
      }
      /**
       * @brief Returns whether or not the pointer points to within this memory block
       *
       * @param ptr pointer to check
       * @return true if `ptr` came from this memory block
       * @return false otherwise
       */
      bool contains(pointer ptr) const noexcept
      {
        return std::less_equal<pointer>()(first, ptr) && std::less<pointer>()(ptr, last);
      }
    };
  }

  /**
   * @brief Calls Resource::allocate when it is unable to fulfil an allocation request up to `Size`
   * times
   *
   * @tparam Bytes size in bytes to be used in the call to Resource::allocate
   * @tparam Alignment alignment in bytes to be used in the call to Resource::allocate
   * @tparam Size maximum number of Resource::allocate calls
   * @tparam Strategy strategy to use to distribute memory from calling Resource::allocate
   * @tparam Resource resource to use for internal allocation requests
   */
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
    /**
     * @brief pointer type
     */
    using pointer = typename Resource::pointer;
    /**
     * @brief size type
     */
    using size_type = typename Resource::size_type;
    /**
     * @brief memory block type
     */
    using mem_block = cascade_detail::mem_block<pointer, size_type>;

  public: // constructors
    /**
     * @brief Construct a new cascade object
     */
    cascade() = default;
    /**
     * @brief Deleted copy constructor since cascade is resource manager
     */
    cascade(cascade const &) = delete;
    /**
     * @brief Deleted copy assignment operator since cascade is a resource manager
     */
    cascade & operator=(cascade const &) = delete;
    /**
     * @brief Destroy the cascade object
     *
     * @note Deallocates any memory allocated back to Resource
     */
    ~cascade() noexcept
    {
      clear();
    }

  public: // modifiers
    /**
     * @copydoc Resource::allocate
     *
     * @note Will call Resource::allocate if all Strategies are unable to fulfil request.
     */
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
    /**
     * @copydoc Resource::deallocate
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      (*this)[ptr].second.deallocate(ptr, bytes, alignment);
    }

  public: // observers
    /**
     * @brief Returns a pair of references to the owning element. Must be a pointer to somewhere
     * inside of an allocated block.
     *
     * @param ptr pointer that points to within an allocated block
     */
    std::pair<mem_block const &, Strategy &> operator[](pointer ptr) noexcept
    {
      auto i = find(ptr);
      assert(i != length);
      return {mem_blocks()[i], strategies()[i]};
    }
    /**
     * @copydoc cascade::operator[]
     */
    std::pair<mem_block const &, Strategy const &> operator[](pointer ptr) const noexcept
    {
      auto i = find(ptr);
      assert(i != length);
      return {mem_blocks()[i], strategies()[i]};
    }

  private: // operator[] helper
    /**
     * @brief Returns the index of mem_block to which the ptr belongs.
     *
     * @param ptr pointer to within an allocated block
     * @return std::size_t index of mem_block
     * @note should never be `length`
     */
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
    /**
     * @brief Add a mem_block and `Strategy` to the end of our containers. Calls Resource::allocate.
     *
     * @return pointer to the added `Strategy` if successful
     * @return `nullptr` if unsuccessful
     */
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
    /**
     * @brief Removes all elements from our containers. Calls Resource::deallocate.
     */
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
    /**
     * @brief Returns a pointer to the start of our `mem_block` container
     */
    mem_block * mem_blocks() noexcept
    {
      return reinterpret_cast<mem_block *>(&mem_block_storage);
    }
    /**
     * @brief Returns a pointer to the start of our `mem_block` container
     */
    mem_block const * mem_blocks() const noexcept
    {
      return reinterpret_cast<mem_block const *>(&mem_block_storage);
    }
    /**
     * @brief Returns a pointer to the start of our `Strategy` container
     */
    Strategy * strategies() noexcept
    {
      return reinterpret_cast<Strategy *>(&strategy_storage);
    }
    /**
     * @brief Returns a pointer to the start of our `Strategy` container
     */
    Strategy const * strategies() const noexcept
    {
      return reinterpret_cast<Strategy const *>(&strategy_storage);
    }

  private: // variables
    /**
     * @brief number of elements we have in our containers. Range is [0, `Size`].
     */
    std::size_t length = 0;
    /**
     * @brief `Strategy` container
     */
    std::aligned_storage_t<sizeof(Strategy) * Size, alignof(Strategy)> strategy_storage;
    /**
     * @brief `mem_block` container
     */
    std::aligned_storage_t<sizeof(mem_block) * Size, alignof(mem_block)> mem_block_storage;
    /**
     * @brief `Resource`
     */
    Resource resource;
  };
}
