#include "free_block.h"

#include "heap.h" // heap
#include "stack.h" // stack
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  free_block<32, 4, 2, stack<4>, heap> m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  REQUIRE(a != b);
  REQUIRE(m[a] == m[b]);
  auto c = m.allocate(32, 4);
  REQUIRE(c != nullptr);
  REQUIRE(b != c);
  REQUIRE(m[b] == m[c]);
  // replicate
  auto d = m.allocate(32, 4);
  REQUIRE(d != nullptr);
  REQUIRE(m[a] != m[d]);
  SECTION("deallocate recovers with stack functionality")
  {
    m.deallocate(c, 32, 4);
    m.deallocate(b, 64, 4);
    m.deallocate(a, 32, 4);

    auto e = m.allocate(32, 4);
    REQUIRE(e != nullptr);
    REQUIRE(e == a);
    auto f = m.allocate(96, 4);
    REQUIRE(f != nullptr);
    REQUIRE(f == b);
  }
  SECTION("exhausted memory and replicas")
  {
    auto e = m.allocate(128, 4);
    REQUIRE(e == nullptr);
  }

  m.deallocate(d, 32, 4);
  m.deallocate(c, 32, 4);
  m.deallocate(b, 64, 4);
  m.deallocate(a, 32, 4);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<free_block<32, 4, 1, stack<4>, heap>> == true);
}