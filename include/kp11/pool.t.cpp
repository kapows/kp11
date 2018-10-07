#include "pool.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("size", "[size]")
{
  SECTION("1")
  {
    pool<10> m;
    REQUIRE(m.size() == 10);
  }
  SECTION("2")
  {
    pool<101581> m;
    REQUIRE(m.size() == 101581);
  }
}
TEST_CASE("set", "[set]")
{
  pool<10> m;
  SECTION("success")
  {
    auto a = m.set(1);
    REQUIRE(a == 0);
    SECTION("post condition")
    {
      auto b = m.set(1);
      REQUIRE(b == 1);
      REQUIRE(b != a);
    }
  }
  SECTION("failure")
  {
    for (auto i = 0; i < m.size(); ++i)
    {
      m.set(1);
    }
    REQUIRE(m.set(1) == m.size());
  }
}
TEST_CASE("reset", "[reset]")
{
  pool<10> m;
  SECTION("recovers indexes")
  {
    auto a = m.set(1);
    m.reset(a, 1);
    auto b = m.set(1);
    REQUIRE(b == a);
  }
  SECTION("accepts size() in reset")
  {
    for (auto i = 0; i < m.size(); ++i)
    {
      m.set(1);
    }
    auto b = m.set(1);
    REQUIRE(b == m.size());
    m.reset(b, 1);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<pool<10>> == true);
}