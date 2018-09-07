#pragma once

#include "traits.h" // is_resource_v

#include <memory_resource> // pmr::memory_resource

namespace kp11
{
  template<typename Resource>
  class memory_resource : public Resource, public std::pmr::memory_resource
  {
    static_assert(is_resource_v<Resource>);

  public: // inherited methods
    using Resource::Resource;
    using std::pmr::memory_resource::allocate;
    using std::pmr::memory_resource::deallocate;
    using std::pmr::memory_resource::is_equal;

  private: // pmr overrides
    void * do_allocate(std::size_t bytes, std::size_t align) override
    {
      return Resource::allocate(bytes, align);
    }
    void do_deallocate(void * p, std::size_t bytes, std::size_t align) override
    {
      Resource::deallocate(p, bytes, align);
    }
    bool do_is_equal(const std::pmr::memory_resource & other) const noexcept override
    {
      return this == &other;
    }
  };
}