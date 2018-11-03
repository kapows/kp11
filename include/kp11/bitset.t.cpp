#include "bitset.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("size", "[size]")
{
  SECTION("1")
  {
    bitset<10> m;
    REQUIRE(m.size() == 10);
    REQUIRE(m.max_size() == 10);
    REQUIRE(m.count() == 0);
  }
  SECTION("2")
  {
    bitset<101581> m;
    REQUIRE(m.size() == 101581);
    REQUIRE(m.max_size() == 101581);
    REQUIRE(m.count() == 0);
  }
}
TEST_CASE("allocate", "[allocate]")
{
  bitset<10> m;
  SECTION("allocate 1")
  {
    auto a = m.allocate(1);
    REQUIRE(a == 0);
    REQUIRE(m.count() == 1);
    SECTION("post condition")
    {
      auto b = m.allocate(1);
      REQUIRE(b == 1);
      REQUIRE(b != a);
      REQUIRE(m.count() == 2);
    }
  }
  SECTION("allocate many")
  {
    auto a = m.allocate(5);
    REQUIRE(a == 0);
    REQUIRE(m.count() == 5);
    SECTION("post condition")
    {
      auto b = m.allocate(5);
      REQUIRE(b == 5);
      REQUIRE(b != a);
      REQUIRE(m.count() == 10);
    }
  }
  SECTION("failure")
  {
    m.allocate(10);
    SECTION("one")
    {
      REQUIRE(m.allocate(1) == m.size());
    }
    SECTION("many")
    {
      REQUIRE(m.allocate(5) == m.size());
    }
  }
}
TEST_CASE("deallocate", "[deallocate]")
{
  bitset<10> m;
  auto a = m.allocate(5);
  SECTION("recovers indexes")
  {
    m.deallocate(a, 5);
    REQUIRE(m.count() == 0);
    auto b = m.allocate(10);
    REQUIRE(b == a);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<bitset<10>> == true);
}