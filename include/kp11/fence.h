#pragma once

#include "utility.h" // mem_block

namespace kp11
{
  template<typename Strategy>
  class fence : public Strategy
  {
  public: // typedefs
    using typename Strategy::pointer;
    using typename Strategy::size_type;
    using mem_block = mem_block<pointer, size_type>;

  public: // constructors
    fence(pointer ptr, size_type bytes, size_type alignment) noexcept :
        Strategy(ptr, bytes, alignment), blk(ptr, bytes)
    {
    }

  public: // modifiers
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      return false;
    }

  public: // accessors
    mem_block const & get_mem_block() noexcept
    {
      return blk;
    }

  private: // variables
    mem_block blk;
  };
}