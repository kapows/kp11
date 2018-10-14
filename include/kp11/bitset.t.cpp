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
    REQUIRE(m.size() == 0);
  }
  SECTION("2")
  {
    bitset<101581> m;
    REQUIRE(m.max_size() == 101581);
    REQUIRE(m.size() == 0);
  }
}
TEST_CASE("biggest", "[biggest]")
{
  bitset<10> m;
  SECTION("initial")
  {
    REQUIRE(m.biggest() == 10);
  }
  SECTION("end unset")
  {
    [[maybe_unused]] auto a = m.set(3);
    REQUIRE(m.biggest() == 7);
  }
  SECTION("start unset")
  {
    auto a = m.set(3);
    [[maybe_unused]] auto b = m.set(7);
    m.reset(a, 3);
    REQUIRE(m.biggest() == 3);
  }
  SECTION("middle unset")
  {
    [[maybe_unused]] auto a = m.set(3);
    auto b = m.set(4);
    [[maybe_unused]] auto c = m.set(3);
    m.reset(b, 4);
    REQUIRE(m.biggest() == 4);
  }
  SECTION("merges")
  {
    auto a = m.set(3);
    auto b = m.set(4);
    auto c = m.set(3);
    m.reset(a, 3);
    m.reset(b, 4);
    REQUIRE(m.biggest() == 7);
    m.reset(c, 3);
    REQUIRE(m.biggest() == 10);
  }
}
TEST_CASE("set", "[set]")
{
  bitset<10> m;
  SECTION("set 1")
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
  SECTION("set many")
  {
    auto a = m.set(5);
    REQUIRE(a == 0);
    REQUIRE(m.size() == 5);
    SECTION("post condition")
    {
      auto b = m.set(5);
      REQUIRE(b == 5);
      REQUIRE(b != a);
      REQUIRE(m.size() == 10);
    }
  }
}
TEST_CASE("reset", "[reset]")
{
  bitset<10> m;
  auto a = m.set(5);
  SECTION("recovers indexes")
  {
    m.reset(a, 5);
    REQUIRE(m.size() == 0);
    auto b = m.set(10);
    REQUIRE(b == a);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<bitset<10>> == true);
}