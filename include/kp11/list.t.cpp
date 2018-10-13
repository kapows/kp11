#include "list.h"

#include "traits.h" // is_marker_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("max_size", "[max_size]")
{
  SECTION("1")
  {
    list<10> m;
    REQUIRE(m.max_size() == 10);
    REQUIRE(m.size() == m.max_size());
  }
  SECTION("2")
  {
    list<101> m;
    REQUIRE(m.max_size() == 101);
    REQUIRE(m.size() == m.max_size());
  }
}
TEST_CASE("biggest", "[biggest]")
{
  list<10> m;
  SECTION("initial")
  {
    REQUIRE(m.biggest() == 10);
  }
  SECTION("out of spots")
  {
    [[maybe_unused]] auto a = m.set(10);
    REQUIRE(m.biggest() == 0);
  }
  SECTION("set")
  {
    [[maybe_unused]] auto a = m.set(3);
    auto b = m.set(4);
    [[maybe_unused]] auto c = m.set(3);
    m.reset(b, 4);
    REQUIRE(m.biggest() == 4);
  }
  SECTION("merge")
  {
    auto a = m.set(3);
    auto b = m.set(4);
    auto c = m.set(3);
    m.reset(b, 4);
    m.reset(c, 3);
    REQUIRE(m.biggest() == 7);
    m.reset(a, 3);
    REQUIRE(m.biggest() == 10);
  }
}
TEST_CASE("set", "[set]")
{
  list<10> m;
  SECTION("single free list node")
  {
    SECTION("all vacant")
    {
      SECTION("less than size")
      {
        REQUIRE(m.biggest() >= 5);
        [[maybe_unused]] auto a = m.set(5);
        REQUIRE(m.size() == 5);
      }
      SECTION("exact size")
      {
        REQUIRE(m.biggest() >= 10);
        [[maybe_unused]] auto a = m.set(10);
        REQUIRE(m.size() == 0);
      }
    }
    SECTION("some occupied")
    {
      auto a = m.set(3);
      SECTION("less than size")
      {
        REQUIRE(m.biggest() >= 3);
        auto b = m.set(3);
        REQUIRE(b != a);
        REQUIRE(m.size() == 4);
      }
      SECTION("exact size")
      {
        REQUIRE(m.biggest() >= 7);
        auto b = m.set(7);
        REQUIRE(b != a);
        REQUIRE(m.size() == 0);
      }
    }
  }
  SECTION("multiple free list nodes")
  {
    auto a = m.set(3);
    [[maybe_unused]] auto b = m.set(3);
    auto c = m.set(4);
    m.reset(a, 3);
    m.reset(c, 4);
    SECTION("less than size")
    {
      REQUIRE(m.biggest() >= 2);
      [[maybe_unused]] auto d = m.set(2);
      REQUIRE(m.size() == 5);
    }
    SECTION("exact size")
    {
      REQUIRE(m.biggest() >= 4);
      [[maybe_unused]] auto d = m.set(4);
      REQUIRE(m.size() == 3);
    }
  }
}
TEST_CASE("reset", "[reset]")
{
  list<10> m;
  SECTION("boundary, boundary")
  {
    auto a = m.set(10);
    m.reset(a, 10);
    REQUIRE(m.size() == 10);
    REQUIRE(m.biggest() == 10);
  }
  SECTION("boundary, occupied")
  {
    auto a = m.set(5);
    [[maybe_unused]] auto b = m.set(5);
    m.reset(a, 5);
    REQUIRE(m.size() == 5);
    REQUIRE(m.biggest() == 5);
  }
  SECTION("occupied, boundary")
  {
    [[maybe_unused]] auto a = m.set(5);
    auto b = m.set(5);
    m.reset(b, 5);
    REQUIRE(m.size() == 5);
    REQUIRE(m.biggest() == 5);
  }
  SECTION("boundary, vacant")
  {
    auto a = m.set(5);
    auto b = m.set(5);
    m.reset(b, 5);
    m.reset(a, 5);
    REQUIRE(m.size() == 10);
    REQUIRE(m.biggest() == 10);
  }
  SECTION("vacant, boundary")
  {
    auto a = m.set(5);
    auto b = m.set(5);
    m.reset(a, 5);
    m.reset(b, 5);
    REQUIRE(m.size() == 10);
    REQUIRE(m.biggest() == 10);
  }
  SECTION("occupied, occupied")
  {
    [[maybe_unused]] auto a = m.set(3);
    auto b = m.set(4);
    [[maybe_unused]] auto c = m.set(3);
    m.reset(b, 4);
    REQUIRE(m.size() == 4);
    REQUIRE(m.biggest() == 4);
  }
  SECTION("vacant, vacant")
  {
    auto a = m.set(3);
    auto b = m.set(4);
    auto c = m.set(3);
    m.reset(a, 3);
    REQUIRE(m.size() == 3);
    m.reset(c, 3);
    REQUIRE(m.size() == 6);
    m.reset(b, 4);
    REQUIRE(m.size() == 10);
    REQUIRE(m.biggest() == 10);
  }
  SECTION("occupied, vacant")
  {
    [[maybe_unused]] auto a = m.set(3);
    auto b = m.set(4);
    auto c = m.set(3);
    m.reset(c, 3);
    REQUIRE(m.size() == 3);
    m.reset(b, 4);
    REQUIRE(m.size() == 7);
    REQUIRE(m.biggest() == 7);
  }
  SECTION("vacant, occupied")
  {
    auto a = m.set(3);
    auto b = m.set(4);
    [[maybe_unused]] auto c = m.set(3);
    m.reset(a, 3);
    REQUIRE(m.size() == 3);
    m.reset(b, 4);
    REQUIRE(m.size() == 7);
    REQUIRE(m.biggest() == 7);
  }
}
TEST_CASE("internal biggest", "[biggest]")
{
  list<10> m;
  auto a = m.set(1);
  auto b = m.set(1);
  auto c = m.set(1);
  [[maybe_unused]] auto d = m.set(1);
  auto e = m.set(1);
  auto f = m.set(1);
  [[maybe_unused]] auto g = m.set(1);
  [[maybe_unused]] auto h = m.set(1);
  [[maybe_unused]] auto i = m.set(1);
  [[maybe_unused]] auto j = m.set(1);
  // empty
  REQUIRE(m.size() == 0);
  // make biggest is 2
  m.reset(e, 1);
  m.reset(f, 1);

  // make biggest is 3
  m.reset(a, 1);
  m.reset(c, 1);
  m.reset(b, 1);

  SECTION("set the biggest")
  {
    [[maybe_unused]] auto k = m.set(3);
  }
  SECTION("set the 2nd biggest")
  {
    [[maybe_unused]] auto k = m.set(2);
  }
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<list<10>> == true);
}