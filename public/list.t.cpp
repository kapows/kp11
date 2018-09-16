#include "list.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("set/reset", "[modifiers]")
{
  list<10> m;
  REQUIRE(m.size() == 10);
  SECTION("make sure size is not fixed")
  {
    list<101581> n;
    REQUIRE(n.size() == 101581);
  }
  REQUIRE(m.set(16) == m.size());
  REQUIRE(m.set(1) == 0);
  REQUIRE(m.set(2) == 1);
  REQUIRE(m.set(3) == 3);
  REQUIRE(m.set(4) == 6);
  REQUIRE(m.set(1) == m.size());

  SECTION("reset allows setting")
  {
    m.reset(3, 3);
    REQUIRE(m.set(3) == 3);
  }
  SECTION("reset merges allow setting")
  {
    m.reset(3, 3);
    m.reset(1, 2);
    REQUIRE(m.set(4) == 1);
    REQUIRE(m.set(1) == 5);
  }
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<list<10>> == true);
}