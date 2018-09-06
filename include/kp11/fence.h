#pragma once

#include "utility.h" // mem_block

namespace kp11
{
  /**
   * @brief Allows a Strategy have knowledge of the ownership of the memory it allocates.
   * Ownership is determined by `mem_block`.
   *
   * @tparam Strategy type that meets the `Strategy` concept
   */
  template<typename Strategy>
  class fence : public Strategy
  {
  public: // typedefs
    using typename Strategy::pointer;
    using typename Strategy::size_type;
    /**
     * @brief mem_block type
     */
    using mem_block = mem_block<pointer, size_type>;

  public: // constructors
    /**
     * @brief Construct a new fence object
     * @copydoc Strategy::Strategy
     */
    fence(pointer ptr, size_type bytes, size_type alignment) noexcept :
        Strategy(ptr, bytes, alignment), blk(ptr, bytes)
    {
    }

  public: // modifiers
    /**
     * @copydoc Strategy::deallocate
     *
     * @return true if fence contains `ptr` and thus has deallocated it
     * @return false otherwise
     */
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (blk.contains(ptr))
      {
        Strategy::deallocate(ptr, bytes, alignment);
        return true;
      }
      return false;
    }

  public: // accessors
    /**
     * @brief Get the mem_block object
     */
    mem_block const & get_mem_block() noexcept
    {
      return blk;
    }

  private: // variables
    mem_block blk;
  };
}