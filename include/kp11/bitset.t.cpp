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
TEST_CASE("max_alloc", "[max_alloc]")
{
  bitset<10> m;
  SECTION("initial")
  {
    REQUIRE(m.max_alloc() == 10);
  }
  SECTION("end unset")
  {
    [[maybe_unused]] auto a = m.allocate(3);
    REQUIRE(m.max_alloc() == 7);
  }
  SECTION("start unset")
  {
    auto a = m.allocate(3);
    [[maybe_unused]] auto b = m.allocate(7);
    m.deallocate(a, 3);
    REQUIRE(m.max_alloc() == 3);
  }
  SECTION("middle unset")
  {
    [[maybe_unused]] auto a = m.allocate(3);
    auto b = m.allocate(4);
    [[maybe_unused]] auto c = m.allocate(3);
    m.deallocate(b, 4);
    REQUIRE(m.max_alloc() == 4);
  }
  SECTION("merges")
  {
    auto a = m.allocate(3);
    auto b = m.allocate(4);
    auto c = m.allocate(3);
    m.deallocate(a, 3);
    m.deallocate(b, 4);
    REQUIRE(m.max_alloc() == 7);
    m.deallocate(c, 3);
    REQUIRE(m.max_alloc() == 10);
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