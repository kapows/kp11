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
    REQUIRE(m.max_size() == 10);
    REQUIRE(m.count() == 0);
  }
  SECTION("2")
  {
    list<101> m;
    REQUIRE(m.size() == 101);
    REQUIRE(m.max_size() == 101);
    REQUIRE(m.count() == 0);
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
        [[maybe_unused]] auto a = m.allocate(5);
        REQUIRE(m.count() == 5);
      }
      SECTION("exact size")
      {
        [[maybe_unused]] auto a = m.allocate(10);
        REQUIRE(m.count() == 10);
      }
    }
    SECTION("some occupied")
    {
      auto a = m.allocate(3);
      SECTION("less than size")
      {
        auto b = m.allocate(3);
        REQUIRE(b != a);
        REQUIRE(m.count() == 6);
      }
      SECTION("exact size")
      {
        auto b = m.allocate(7);
        REQUIRE(b != a);
        REQUIRE(m.count() == 10);
      }
    }
  }
  SECTION("multiple free list nodes")
  {
    auto a = m.allocate(3);
    [[maybe_unused]] auto b = m.allocate(3);
    auto c = m.allocate(4);
    m.deallocate(a, 3);
    m.deallocate(c, 4);
    SECTION("less than size")
    {
      [[maybe_unused]] auto d = m.allocate(2);
      REQUIRE(m.count() == 5);
    }
    SECTION("exact size")
    {
      [[maybe_unused]] auto d = m.allocate(4);
      REQUIRE(m.count() == 7);
    }
  }
  SECTION("failure")
  {
    m.allocate(10);
    REQUIRE(m.allocate(1) == m.size());
  }
}
TEST_CASE("reset", "[reset]")
{
  list<10> m;
  SECTION("boundary, boundary")
  {
    auto a = m.allocate(10);
    m.deallocate(a, 10);
    REQUIRE(m.count() == 0);
  }
  SECTION("boundary, occupied")
  {
    auto a = m.allocate(5);
    [[maybe_unused]] auto b = m.allocate(5);
    m.deallocate(a, 5);
    REQUIRE(m.count() == 5);
  }
  SECTION("occupied, boundary")
  {
    [[maybe_unused]] auto a = m.allocate(5);
    auto b = m.allocate(5);
    m.deallocate(b, 5);
    REQUIRE(m.count() == 5);
  }
  SECTION("boundary, vacant")
  {
    auto a = m.allocate(5);
    auto b = m.allocate(5);
    m.deallocate(b, 5);
    m.deallocate(a, 5);
    REQUIRE(m.count() == 0);
  }
  SECTION("vacant, boundary")
  {
    auto a = m.allocate(5);
    auto b = m.allocate(5);
    m.deallocate(a, 5);
    m.deallocate(b, 5);
    REQUIRE(m.count() == 0);
  }
  SECTION("occupied, occupied")
  {
    [[maybe_unused]] auto a = m.allocate(3);
    auto b = m.allocate(4);
    [[maybe_unused]] auto c = m.allocate(3);
    m.deallocate(b, 4);
    REQUIRE(m.count() == 6);
  }
  SECTION("vacant, vacant")
  {
    auto a = m.allocate(3);
    auto b = m.allocate(4);
    auto c = m.allocate(3);
    m.deallocate(a, 3);
    REQUIRE(m.count() == 7);
    m.deallocate(c, 3);
    REQUIRE(m.count() == 4);
    m.deallocate(b, 4);
    REQUIRE(m.count() == 0);
  }
  SECTION("occupied, vacant")
  {
    [[maybe_unused]] auto a = m.allocate(3);
    auto b = m.allocate(4);
    auto c = m.allocate(3);
    m.deallocate(c, 3);
    REQUIRE(m.count() == 7);
    m.deallocate(b, 4);
    REQUIRE(m.count() == 3);
  }
  SECTION("vacant, occupied")
  {
    auto a = m.allocate(3);
    auto b = m.allocate(4);
    [[maybe_unused]] auto c = m.allocate(3);
    m.deallocate(a, 3);
    REQUIRE(m.count() == 7);
    m.deallocate(b, 4);
    REQUIRE(m.count() == 3);
  }
}
TEST_CASE("best fit", "[bestfit]")
{
  list<10> m;
  auto a = m.allocate(1);
  auto b = m.allocate(1);
  auto c = m.allocate(1);
  [[maybe_unused]] auto d = m.allocate(1);
  auto e = m.allocate(1);
  auto f = m.allocate(1);
  [[maybe_unused]] auto g = m.allocate(1);
  [[maybe_unused]] auto h = m.allocate(1);
  [[maybe_unused]] auto i = m.allocate(1);
  [[maybe_unused]] auto j = m.allocate(1);
  // empty
  REQUIRE(m.count() == 10);

  m.deallocate(a, 1);
  m.deallocate(b, 1);
  m.deallocate(c, 1);

  m.deallocate(e, 1);
  m.deallocate(f, 1);

  SECTION("set the best fit")
  {
    [[maybe_unused]] auto k = m.allocate(2);
  }
}
TEST_CASE("max separate", "[max_separate]")
{
  list<11> m;
  auto a = m.allocate(1);
  [[maybe_unused]] auto b = m.allocate(1);
  auto c = m.allocate(1);
  [[maybe_unused]] auto d = m.allocate(1);
  auto e = m.allocate(1);
  [[maybe_unused]] auto f = m.allocate(1);
  auto g = m.allocate(1);
  [[maybe_unused]] auto h = m.allocate(1);
  auto i = m.allocate(1);
  [[maybe_unused]] auto j = m.allocate(1);
  auto k = m.allocate(1);
  m.deallocate(a, 1);
  m.deallocate(c, 1);
  m.deallocate(e, 1);
  m.deallocate(g, 1);
  m.deallocate(i, 1);
  m.deallocate(k, 1);
  REQUIRE(m.count() == 5);
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_marker_v<list<10>> == true);
}