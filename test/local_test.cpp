#include "local.h"

#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  local<128, 4> m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(32, 4);
  REQUIRE(b == nullptr);
  m.deallocate(a, 32, 4);
  auto c = m.allocate(128, 4);
  REQUIRE(c != nullptr);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<local<128, 4>> == true);
}