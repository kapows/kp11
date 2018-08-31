#include "pool.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

#include <limits>

using namespace kp11;

TEST_CASE("size", "[capacity]")
{
  pool<10> m;
  REQUIRE(m.size() == 10);
  pool<101581> n;
  REQUIRE(n.size() == 101581);
}
TEST_CASE("set/reset", "[modifiers]")
{
  pool<10> m;
  REQUIRE(m.set(1) == 0);
  REQUIRE(m.set(1) == 1);
  SECTION("reverse order reset")
  {
    m.reset(1, 1);
    m.reset(0, 1);
    REQUIRE(m.set(1) == 0);
    REQUIRE(m.set(1) == 1);
  }
  SECTION("out of order reset")
  {
    m.reset(0, 1);
    m.reset(1, 1);
    REQUIRE(m.set(1) == 1);
    REQUIRE(m.set(1) == 0);
  }
  SECTION("out of space")
  {
    for (auto i = 2; i < 10; ++i)
    {
      REQUIRE(m.set(1) == i);
    }
    REQUIRE(m.set(1) == m.size());
  }
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<pool<10>> == true);
}