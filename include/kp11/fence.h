#pragma once

#include "traits.h" // is_strategy_v

#include <functional> // less_equal, less
#include <memory> // pointer_traits

namespace kp11
{
  /**
   * @brief Allows a Strategy have knowledge of the ownership of the memory it allocates.
   *
   * @tparam Strategy type that meets the `Strategy` concept
   */
  template<typename Strategy>
  class fence : public Strategy
  {
    static_assert(is_strategy_v<Strategy>, "fence requires Strategy to be a Strategy");

  public: // typedefs
    using typename Strategy::pointer;
    using typename Strategy::size_type;

  public: // constructors
    fence(pointer ptr, size_type bytes, size_type alignment) noexcept :
        Strategy(ptr, bytes, alignment), first(ptr), last(advance(ptr, bytes))
    {
    }

  public: // modifiers
    bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
      if (owns(ptr, bytes, alignment))
      {
        Strategy::deallocate(ptr, bytes, alignment);
        return true;
      }
      return false;
    }

  public: // ownership
    bool owns(pointer ptr, size_type bytes, size_type alignment) const noexcept
    {
      return std::less_equal<pointer>()(first, ptr) && std::less<pointer>()(ptr, last);
    }

  private: // pointer helper
    static pointer advance(pointer ptr, size_type bytes) noexcept
    {
      using char_ptr = typename std::pointer_traits<pointer>::template rebind<char>;
      return static_cast<pointer>(static_cast<char_ptr>(ptr) + bytes);
    }

  private: // variables
    pointer first;
    pointer last;
  };
}