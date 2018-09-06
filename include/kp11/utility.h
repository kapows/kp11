#pragma once

#include <functional> // less, less_equal
#include <memory> // pointer_traits

namespace kp11
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
   * @brief Stores a pointer to the beginning and a pointer to the end of an allocated block of
   * memory.
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
     * @param ptr beginning of a memory block
     * @param bytes size of the memory block
     */
    mem_block(pointer ptr, size_type bytes) noexcept : mem_block(ptr, advance(ptr, bytes))
    {
    }

  public: // observers
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

  public: // operators
    /**
     * @brief Equality operator
     */
    friend bool operator==(mem_block const & lhs, mem_block const & rhs) noexcept
    {
      return lhs.first == rhs.first && lhs.last == rhs.last;
    }
    /**
     * @brief Inequality operator
     */
    friend bool operator!=(mem_block const & lhs, mem_block const & rhs) noexcept
    {
      return !(lhs == rhs);
    }
    /**
     * @brief Less than comparison operator
     */
    friend bool operator<(mem_block const & lhs, mem_block const & rhs) noexcept
    {
      return std::less<pointer>()(lhs.first, rhs.first) && std::less<pointer>()(lhs.last, rhs.last);
    }
  };
}
