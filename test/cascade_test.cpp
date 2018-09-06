#include "cascade.h"

#include "free_block.h" // free_block
#include "heap.h" // heap
#include "stack.h" // stack

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  cascade<128, 4, 2, free_block<32, stack<4>>, heap> m;

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
    REQUIRE(s.first.first == a);
    auto e = s.second.allocate(32, 4);
    REQUIRE(e != nullptr);
    s.second.deallocate(e, 32, 4);

    auto const & n = m;
    auto r = n[static_cast<char *>(a) + 64];
    REQUIRE(r.first == s.first);
    REQUIRE(&r.second == &s.second);
  }
  SECTION("index operator 2")
  {
    auto s = m[static_cast<char *>(b) + 64];
    REQUIRE(s.first.first == b);
    auto e = s.second.allocate(32, 4);
    REQUIRE(e == nullptr);
  }
  m.deallocate(b, 128, 4);
  m.deallocate(a, 96, 4);
}