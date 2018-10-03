#include "pool.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  pool<10> m;
  REQUIRE(m.size() == 10);
  SECTION("make sure size is not fixed")
  {
    pool<101581> n;
  }
  for (auto i = 0; i < 10; ++i)
  {
    REQUIRE(m.set(1) == i);
  }
  REQUIRE(m.set(1) == m.size());
  SECTION("reset allows setting in LIFO order")
  {
    m.reset(8, 1);
    m.reset(2, 1);
    m.reset(4, 1);
    REQUIRE(m.set(1) == 4);
    REQUIRE(m.set(1) == 2);
    REQUIRE(m.set(1) == 8);
  }
  SECTION("accepts size() in reset")
  {
    m.reset(m.size(), 1);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<pool<10>> == true);
}
