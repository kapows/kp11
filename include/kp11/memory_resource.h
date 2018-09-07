#pragma once

#include <memory_resource> // pmr::memory_resource

namespace kp11
{
  template<typename Resource>
  class memory_resource : public std::pmr::memory_resource
  {
  private: // pmr overrides
    void * do_allocate(std::size_t bytes, std::size_t align) override
    {
      return nullptr;
    }
    void do_deallocate(void * p, std::size_t bytes, std::size_t align) override
    {
    }
    bool do_is_equal(const std::pmr::memory_resource & other) const noexcept override
    {
      return false;
    }
  };
}