#include "heap.h"

#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate/deallocate", "[modifiers]")
{
  heap m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  m.deallocate(a, 32, 4);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<heap> == true);
}