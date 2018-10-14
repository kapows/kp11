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
    REQUIRE(m.size() == 0);
  }
  SECTION("2")
  {
    pool<101581> m;
    REQUIRE(m.max_size() == 101581);
    REQUIRE(m.size() == 0);
  }
}
TEST_CASE("biggest", "[biggest]")
{
  pool<10> m;
  SECTION("not empty")
  {
    REQUIRE(m.biggest() == 1);
  }
  SECTION("empty")
  {
    for (auto i = 0; i < 10; ++i)
    {
      m.set(1);
    }
    REQUIRE(m.biggest() == 0);
  }
}
TEST_CASE("set", "[set]")
{
  pool<10> m;
  SECTION("success")
  {
    auto a = m.set(1);
    REQUIRE(a == 0);
    REQUIRE(m.size() == 1);
    SECTION("post condition")
    {
      auto b = m.set(1);
      REQUIRE(b == 1);
      REQUIRE(b != a);
      REQUIRE(m.size() == 2);
    }
  }
}
TEST_CASE("reset", "[reset]")
{
  pool<10> m;
  SECTION("recovers indexes")
  {
    auto a = m.set(1);
    m.reset(a, 1);
    REQUIRE(m.size() == 0);
    auto b = m.set(1);
    REQUIRE(b == a);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<pool<10>> == true);
}