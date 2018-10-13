#include "stack.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("max_size", "[max_size]")
{
  SECTION("1")
  {
    stack<10> m;
    REQUIRE(m.max_size() == 10);
    REQUIRE(m.size() == 10);
  }
  SECTION("2")
  {
    stack<101581> m;
    REQUIRE(m.max_size() == 101581);
    REQUIRE(m.size() == 101581);
  }
}
TEST_CASE("biggest", "[biggest]")
{
  stack<10> m;
  SECTION("initial")
  {
    REQUIRE(m.biggest() == 10);
  }
  SECTION("empty")
  {
    [[maybe_unused]] auto a = m.set(10);
    REQUIRE(m.biggest() == 0);
  }
  SECTION("set")
  {
    [[maybe_unused]] auto a = m.set(3);
    REQUIRE(m.biggest() == 7);
  }
  SECTION("reset")
  {
    auto a = m.set(3);
    [[maybe_unused]] auto b = m.set(7);
    m.reset(a, 3);
    REQUIRE(m.biggest() == 10);
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
    m.reset(a, 3);
    m.reset(b, 4);
    REQUIRE(m.biggest() == 7);
    auto c = m.set(3);
    m.reset(c, 3);
    REQUIRE(m.biggest() == 10);
  }
}
TEST_CASE("set", "[set]")
{
  stack<10> m;
  SECTION("success")
  {
    auto a = m.set(5);
    REQUIRE(a == 0);
    REQUIRE(m.size() == 5);
    SECTION("post condition")
    {
      auto b = m.set(5);
      REQUIRE(b == 5);
      REQUIRE(b != a);
      REQUIRE(m.size() == 0);
    }
  }
  SECTION("failure")
  {
    REQUIRE(m.set(16) == m.max_size());
    REQUIRE(m.size() == 10);
  }
}
TEST_CASE("reset", "[reset]")
{
  stack<10> m;
  SECTION("recovers indexes")
  {
    auto a = m.set(5);
    m.reset(a, 5);
    REQUIRE(m.size() == 10);
    auto b = m.set(10);
    REQUIRE(b == a);
  }
  SECTION("doesn't recover indexes")
  {
    auto a = m.set(3);
    m.set(4);
    m.reset(a, 3);
    REQUIRE(m.size() == 3);
    auto c = m.set(3);
    REQUIRE(c != m.max_size());
    REQUIRE(c != a);
  }
  SECTION("accepts max_size() in reset")
  {
    auto b = m.set(16);
    REQUIRE(b == m.max_size());
    m.reset(b, 16);
    REQUIRE(m.size() == 10);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<stack<10>> == true);
}