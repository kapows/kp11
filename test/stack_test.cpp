#include "stack.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("methods", "[methods]")
{
  stack<10> m;
  REQUIRE(m.size() == 10);
  stack<101581> n;
  REQUIRE(n.size() == 101581);
  REQUIRE(m.set(1) == 0);
  REQUIRE(m.set(11) == m.size());
  m.reset(0, 1);
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<stack<10>> == true);
}
SCENARIO("stack sets spots in increasing order")
{
  stack<10> m;
  GIVEN("all vacant spots")
  {
    WHEN("setting spots")
    {
      THEN("spots are set in ascending order")
      {
        REQUIRE(m.set(1) == 0);
        REQUIRE(m.set(2) == 1);
        REQUIRE(m.set(4) == 3);
      }
    }
  }
  GIVEN("all spots are taken")
  {
    REQUIRE(m.set(10) == 0);
    WHEN("setting spots")
    {
      THEN("spots are not set")
      {
        REQUIRE(m.set(1) == m.size());
        REQUIRE(m.set(2) == m.size());
        REQUIRE(m.set(4) == m.size());
      }
    }
  }
  GIVEN("some spots have been recovered")
  {
    for (auto i = 0; i < 9; ++i)
    {
      REQUIRE(m.set(1) != m.size());
    }
    m.reset(7, 1);
    m.reset(8, 1);
    m.reset(6, 1);
    WHEN("setting spots")
    {
      THEN("spots are set in ascending order")
      {
        REQUIRE(m.set(1) == 8);
        REQUIRE(m.set(1) == 9);
      }
    }
  }
}
SCENARIO("stack recovers spots")
{
  stack<10> m;
  GIVEN("some spots are taken")
  {
    for (auto i = 0; i < 9; ++i)
    {
      REQUIRE(m.set(1) != m.size());
    }
    WHEN("resetting in proper order")
    {
      m.reset(8, 1);
      m.reset(7, 1);
      m.reset(6, 1);
      THEN("spots are recovered")
      {
        REQUIRE(m.set(1) == 6);
        REQUIRE(m.set(1) == 7);
        REQUIRE(m.set(1) == 8);
      }
    }
    WHEN("resetting in the wrong order")
    {
      m.reset(6, 1);
      m.reset(7, 1);
      m.reset(8, 1);
      THEN("spots are not recovered")
      {
        REQUIRE(m.set(1) == 8);
        REQUIRE(m.set(1) == 9);
        REQUIRE(m.set(1) == m.size());
      }
    }
  }
}
