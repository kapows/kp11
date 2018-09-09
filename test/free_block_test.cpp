#include "free_block.h"

#include "heap.h" // heap
#include "stack.h" // stack
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  free_block<32, 4, 1, stack<4>, heap> m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  auto c = m.allocate(32, 4);
  REQUIRE(c != nullptr);
  auto d = m.allocate(32, 4);
  REQUIRE(d == nullptr);

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
}

TEST_CASE("cascade test", "[unit-test]")
{
  free_block<32, 4, 2, stack<4>, heap> m;

  auto a = m.allocate(96, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(128, 4);
  REQUIRE(b != nullptr);
  REQUIRE(a != b);
  SECTION("exhausted memory")
  {
    auto c = m.allocate(128, 4);
    REQUIRE(c == nullptr);
  }
  SECTION("index operator")
  {
    auto s = m[static_cast<char *>(a) + 64];
    REQUIRE(s == a);

    auto const & n = m;
    auto r = n[static_cast<char *>(a) + 64];
    REQUIRE(r == s);
  }
  SECTION("index operator 2")
  {
    auto s = m[static_cast<char *>(b) + 64];
    REQUIRE(s == b);
  }
  m.deallocate(b, 128, 4);
  m.deallocate(a, 96, 4);
  SECTION("memory recovered")
  {
    REQUIRE(m.allocate(96, 4) == a);
  }
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<free_block<32, 4, 1, stack<4>, heap>> == true);
}