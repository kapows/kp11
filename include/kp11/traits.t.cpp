#include "traits.h"

#include <catch.hpp>

#include <cstddef> // size_t

using namespace kp11;

class test_resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept;
  void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
};

class test_not_a_resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept;
  void deallocate(size_type bytes, size_type alignment) noexcept;
};
TEST_CASE("is_resource", "[modifiers]")
{
  REQUIRE(is_resource_v<int> == false);
  REQUIRE(is_resource_v<float> == false);
  REQUIRE(is_resource_v<test_not_a_resource> == false);
  REQUIRE(is_resource_v<test_resource> == true);
}

template<std::size_t N>
class test_marker
{
public:
  using size_type = std::size_t;
  static constexpr size_type size() noexcept
  {
    return N;
  }
  size_type set(size_type n) noexcept;
  void reset(size_type index, size_type n) noexcept;
};

template<std::size_t N>
class test_not_a_marker
{
public:
  using size_type = std::size_t;
  size_type set(size_type n) noexcept;
  void reset(size_type index, size_type n) noexcept;
};
TEST_CASE("is_marker", "[traits]")
{
  REQUIRE(is_marker_v<int> == false);
  REQUIRE(is_marker_v<float> == false);
  REQUIRE(is_marker_v<test_not_a_marker<1>> == false);
  REQUIRE(is_marker_v<test_marker<1>> == true);
}