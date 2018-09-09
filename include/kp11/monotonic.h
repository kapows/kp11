#pragma once

#include "traits.h" // is_resource_v

namespace kp11
{
  /**
   * @brief Allocates memory by incrementing a pointer.
   *
   * @tparam Bytes size of memory in bytes
   * @tparam Alignment alignment of memory in bytes
   * @tparam Replicas number of times to replicate
   * @tparam Upstream type that meets the `Resource` concept. This is where memory will be allocated
   * from.
   */
  template<std::size_t Bytes, std::size_t Alignment, std::size_t Replicas, typename Upstream>
  class monotonic : public Upstream
  {
    static_assert(is_resource_v<Upstream>);

  public: // typedefs
    using typename Upstream::pointer;
    using typename Upstream::size_type;

  public: // constructors
    using Upstream::Upstream;

  public: // modifiers
    pointer allocate(size_type bytes, size_type alignment) noexcept
    {
      return nullptr;
    }
    /**
     * @copydoc Resource::deallocate
     *
     * @par Complexity
     * `O(0)`
     *
     * @note No-op
     */
    void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
    {
    }

  public: // observers
    pointer operator[](pointer ptr) const noexcept
    {
      return nullptr;
    }
  };
}