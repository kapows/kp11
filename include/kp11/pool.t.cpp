#include "pool.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("max_size", "[max_size]")
{
  SECTION("1")
  {
    pool<10> m;
    REQUIRE(m.max_size() == 10);
    REQUIRE(m.size() == m.max_size());
  }
  SECTION("2")
  {
    pool<101581> m;
    REQUIRE(m.max_size() == 101581);
    REQUIRE(m.size() == m.max_size());
  }
}
TEST_CASE("set", "[set]")
{
  pool<10> m;
  SECTION("success")
  {
    auto a = m.set(1);
    REQUIRE(a == 0);
    REQUIRE(m.size() == 9);
    SECTION("post condition")
    {
      auto b = m.set(1);
      REQUIRE(b == 1);
      REQUIRE(b != a);
      REQUIRE(m.size() == 8);
    }
  }
  SECTION("failure")
  {
    for (auto i = 0; i < m.max_size(); ++i)
    {
      m.set(1);
    }
    REQUIRE(m.set(1) == m.max_size());
    REQUIRE(m.size() == 0);
  }
}
TEST_CASE("reset", "[reset]")
{
  pool<10> m;
  SECTION("recovers indexes")
  {
    auto a = m.set(1);
    m.reset(a, 1);
    REQUIRE(m.size() == 10);
    auto b = m.set(1);
    REQUIRE(b == a);
  }
  SECTION("accepts max_size() in reset")
  {
    for (auto i = 0; i < m.max_size(); ++i)
    {
      m.set(1);
    }
    auto b = m.set(1);
    REQUIRE(b == m.max_size());
    m.reset(b, 1);
    REQUIRE(m.size() == 0);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<pool<10>> == true);
}