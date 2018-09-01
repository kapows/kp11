#pragma once

#include "traits.h" // is_strategy_v

#include <cstddef> // size_t
#include <memory> // pointer_traits
#include <type_traits> // aligned_storage_t

namespace kp11
{
  /**
   * @brief Local keeps a buffer inside of itself that it passes to `Strategy`.
   *
   * @tparam Bytes size of buffer
   * @tparam Alignment alignment of buffer
   * @tparam Strategy type that meets the `Strategy` concept
   */
  template<std::size_t Bytes, std::size_t Alignment, typename Strategy>
  class local : public Strategy
  {
    static_assert(is_strategy_v<Strategy>, "local requires Strategy to be a Strategy");

  public: // typedefs
    using pointer = typename Strategy::pointer;
    using size_type = typename Strategy::size_type;

  private: // typedefs
    using buffer_type = std::aligned_storage_t<Bytes, Alignment>;
    using buffer_pointer = typename std::pointer_traits<pointer>::template rebind<buffer_type>;
    using buffer_pointer_traits = std::pointer_traits<buffer_pointer>;

  public: // constructors
    /**
     * @brief Construct a new local object
     */
    local() noexcept :
        Strategy(static_cast<pointer>(buffer_pointer_traits::pointer_to(buffer)),
          static_cast<size_type>(Bytes),
          static_cast<size_type>(Alignment))
    {
    }

  private: // variables
    buffer_type buffer;
  };

}