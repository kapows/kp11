#include "stack.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  stack<10> m;
  REQUIRE(m.size() == 10);
  SECTION("make sure that size isn't fixed")
  {
    stack<101581> n;
    REQUIRE(n.size() == 101581);
  }

  REQUIRE(m.set(1) == 0);
  REQUIRE(m.set(2) == 1);
  REQUIRE(m.set(3) == 3);
  REQUIRE(m.set(4) == 6);
  REQUIRE(m.set(1) == m.size());

  SECTION("out of order reset does not recover spots")
  {
    m.reset(0, 1);
    m.reset(1, 2);
    REQUIRE(m.set(1) == m.size());
  }
  SECTION("reverse order reset recovers spots")
  {
    m.reset(6, 4);
    m.reset(3, 3);
    REQUIRE(m.set(1) == 3);
    REQUIRE(m.set(1) == 4);
    REQUIRE(m.set(5) == 5);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<stack<10>> == true);
}