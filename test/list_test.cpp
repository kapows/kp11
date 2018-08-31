#include "list.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("size", "[capacity]")
{
  list<10> m;
  REQUIRE(m.size() == 10);
  list<101581> n;
  REQUIRE(n.size() == 101581);
}
TEST_CASE("set/reset", "[modifiers]")
{
  list<10> m;
  REQUIRE(m.set(16) == m.size());
  REQUIRE(m.set(5) == 0);
  REQUIRE(m.set(5) == 5);
  REQUIRE(m.set(1) == m.size());
  m.reset(5, 5);
  REQUIRE(m.set(2) == 5);
  REQUIRE(m.set(3) == 7);
  m.reset(7, 3);
  m.reset(5, 2);
  m.reset(0, 5);
  REQUIRE(m.set(10) == 0);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<list<10>> == true);
}