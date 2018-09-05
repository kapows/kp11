#include "pool.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

#include <limits>

using namespace kp11;

TEST_CASE("methods", "[methods]")
{
  pool<10> m;
  REQUIRE(m.size() == 10);
  pool<101581> n;
  REQUIRE(n.size() == 101581);
  REQUIRE(m.set(1) == 0);
  REQUIRE(m.set(1) == 1);
  m.reset(0, 1);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<pool<10>> == true);
}

SCENARIO("pool sets spots in LIFO order")
{
  pool<10> m;
  GIVEN("all vacant spots")
  {
    WHEN("setting spots")
    {
      THEN("spots are set in increasing order")
      {
        REQUIRE(m.set(1) == 0);
        REQUIRE(m.set(1) == 1);
        REQUIRE(m.set(1) == 2);
      }
    }
  }
  GIVEN("all spots taken")
  {
    for (auto i = 0; i < 10; ++i)
    {
      REQUIRE(m.set(1) == i);
    }
    WHEN("setting spots")
    {
      THEN("spots are not set")
      {
        REQUIRE(m.set(1) == m.size());
        REQUIRE(m.set(1) == m.size());
      }
    }
  }
  GIVEN("some spots reset")
  {
    for (auto i = 0; i < 10; ++i)
    {
      REQUIRE(m.set(1) == i);
    }
    m.reset(1, 1);
    m.reset(8, 1);
    m.reset(4, 1);
    WHEN("setting spots")
    {
      THEN("spots are set in LIFO order")
      {
        REQUIRE(m.set(1) == 4);
        REQUIRE(m.set(1) == 8);
        REQUIRE(m.set(1) == 1);
      }
    }
  }
}
