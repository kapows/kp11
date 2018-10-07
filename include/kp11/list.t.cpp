#include "list.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("size", "[size]")
{
  SECTION("1")
  {
    list<10> m;
    REQUIRE(m.size() == 10);
  }
  SECTION("2")
  {
    list<101581> m;
    REQUIRE(m.size() == 101581);
  }
}
TEST_CASE("set", "[set]")
{
  list<10> m;
  SECTION("success")
  {
    auto a = m.set(5);
    REQUIRE(a == 0);
    SECTION("post condition")
    {
      auto b = m.set(5);
      REQUIRE(b == 5);
      REQUIRE(b != a);
    }
  }
  SECTION("failure")
  {
    REQUIRE(m.set(16) == m.size());
  }
}
TEST_CASE("reset", "[reset]")
{
  list<10> m;
  auto a = m.set(5);
  SECTION("recovers indexes")
  {
    m.reset(a, 5);
    auto b = m.set(10);
    REQUIRE(b == a);
  }
  SECTION("accepts size() in reset")
  {
    auto b = m.set(16);
    REQUIRE(b == m.size());
    m.reset(b, 16);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<list<10>> == true);
}