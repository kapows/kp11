#include "heap.h"

#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  heap m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(64, 8);
  REQUIRE(b != nullptr);
  REQUIRE(b != a);
  m.deallocate(a, 32, 4);
  m.deallocate(b, 64, 8);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<heap> == true);
}