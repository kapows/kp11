#include "stack.h"

#include <catch.hpp>

#include <limits>

using namespace kp11;

TEST_CASE("constructor", "[constructor]")
{
  stack m(10);
  stack n(101581);
}
TEST_CASE("size", "[capacity]")
{
  stack m(10);
  REQUIRE(m.size() == 10);
  stack n(101581);
  REQUIRE(n.size() == 101581);
}
TEST_CASE("set/reset", "[modifiers]")
{
  stack m(10);
  REQUIRE(m.set(16) == m.size());
  REQUIRE(m.set(5) == 0);
  REQUIRE(m.set(5) == 5);
  SECTION("proper order reset")
  {
    m.reset(5, 5);
    REQUIRE(m.set(2) == 5);
    REQUIRE(m.set(3) == 7);
  }
  SECTION("out of order reset")
  {
    m.reset(0, 5);
    REQUIRE(m.set(2) == m.size());
    REQUIRE(m.set(3) == m.size());
  }
}