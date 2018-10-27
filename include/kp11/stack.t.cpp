#include "stack.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("size", "[size]")
{
  SECTION("1")
  {
    stack<10> m;
    REQUIRE(m.size() == 10);
    REQUIRE(m.count() == 0);
  }
  SECTION("2")
  {
    stack<101581> m;
    REQUIRE(m.size() == 101581);
    REQUIRE(m.count() == 0);
  }
}
TEST_CASE("max_alloc", "[max_alloc]")
{
  stack<10> m;
  SECTION("initial")
  {
    REQUIRE(m.max_alloc() == 10);
  }
  SECTION("empty")
  {
    [[maybe_unused]] auto a = m.allocate(10);
    REQUIRE(m.max_alloc() == 0);
  }
  SECTION("set")
  {
    [[maybe_unused]] auto a = m.allocate(3);
    REQUIRE(m.max_alloc() == 7);
  }
  SECTION("reset")
  {
    auto a = m.allocate(3);
    auto b = m.allocate(7);
    m.deallocate(b, 7);
    m.deallocate(a, 3);
    REQUIRE(m.max_alloc() == 10);
  }
}
TEST_CASE("allocate", "[allocate]")
{
  stack<10> m;
  SECTION("success")
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
  stack<10> m;
  SECTION("recovers indexes")
  {
    auto a = m.allocate(5);
    m.deallocate(a, 5);
    REQUIRE(m.count() == 0);
    auto b = m.allocate(10);
    REQUIRE(b == a);
  }
  SECTION("doesn't recover indexes")
  {
    auto a = m.allocate(3);
    m.allocate(4);
    m.deallocate(a, 3);
    REQUIRE(m.count() == 7);
    auto c = m.allocate(3);
    REQUIRE(c != a);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<stack<10>> == true);
}