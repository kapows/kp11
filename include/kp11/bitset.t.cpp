#include "bitset.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("size", "[size]")
{
  SECTION("1")
  {
    bitset<10> m;
    REQUIRE(m.max_size() == 10);
  }
  SECTION("2")
  {
    bitset<101581> m;
    REQUIRE(m.max_size() == 101581);
  }
}
TEST_CASE("set", "[set]")
{
  bitset<10> m;
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
    REQUIRE(m.set(16) == m.max_size());
  }
}
TEST_CASE("reset", "[reset]")
{
  bitset<10> m;
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
    REQUIRE(b == m.max_size());
    m.reset(b, 16);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<bitset<10>> == true);
}