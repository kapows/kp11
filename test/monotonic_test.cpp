#include "monotonic.h"

#include "heap.h" // heap
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  monotonic<128, 4, 2, heap> m;
  auto a = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  REQUIRE(m[a] != nullptr);
  m.deallocate(a, 128, 4);
  auto b = m.allocate(128, 4);
  REQUIRE(b != nullptr);
  REQUIRE(a != b);
  REQUIRE(m[b] != nullptr);
  REQUIRE(m[a] != m[b]);
  m.deallocate(b, 128, 4);
  auto c = m.allocate(128, 4);
  REQUIRE(c == nullptr);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<monotonic<128, 4, 2, heap>> == true);
}