#include "fallback.h"

#include "free_block.h" // free_block
#include "local.h" // local
#include "monotonic.h" // monotonic
#include "stack.h" // stack

#include <catch.hpp>

#include <tuple> // forward_as_tuple

using namespace kp11;

TEST_CASE("constructor", "[constructor]")
{
  SECTION("default")
  {
    fallback<local<128, 4>, local<128, 4>> m;
  }
  SECTION("forwarding")
  {
    fallback<free_block<1, stack<4>, local<128, 4>>, local<128, 4>> m(
      std::piecewise_construct, std::forward_as_tuple(128, 4), std::forward_as_tuple());
  }
}
TEST_CASE("accessor", "[accessor]")
{
  fallback<local<128, 4>, local<128, 4>> m;
  [[maybe_unused]] auto & a = m.get_primary();
  [[maybe_unused]] auto & b = m.get_secondary();
  auto const & n = m;
  [[maybe_unused]] auto & c = n.get_primary();
  [[maybe_unused]] auto & d = n.get_secondary();
}
TEST_CASE("allocate", "[allocate]")
{
  fallback<free_block<1, stack<4>, local<128, 4>>, local<128, 4>> m(
    std::piecewise_construct, std::forward_as_tuple(128, 4), std::forward_as_tuple());
  auto a = m.allocate(64, 4);
  REQUIRE(a != nullptr);
  REQUIRE(m.get_primary()[a] != nullptr);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  REQUIRE(m.get_primary()[b] != nullptr);
  SECTION("secondary")
  {
    auto c = m.allocate(64, 4);
    REQUIRE(c != nullptr);
    REQUIRE(m.get_primary()[c] == nullptr);
    SECTION("out of memory")
    {
      auto d = m.allocate(64, 4);
      REQUIRE(d == nullptr);
    }
  }
}
TEST_CASE("deallocate", "[deallocate]")
{
  SECTION("returns convertible bool")
  {
    fallback<free_block<1, stack<4>, local<128, 4>>, local<128, 4>> m(
      std::piecewise_construct, std::forward_as_tuple(128, 4), std::forward_as_tuple());
    auto a = m.allocate(64, 4);
    REQUIRE(a != nullptr);
    REQUIRE(m.get_primary()[a] != nullptr);
    auto b = m.allocate(64, 4);
    REQUIRE(b != nullptr);
    REQUIRE(m.get_primary()[b] != nullptr);
    SECTION("secondary")
    {
      auto c = m.allocate(64, 4);
      REQUIRE(c != nullptr);
      REQUIRE(m.get_primary()[c] == nullptr);
      m.deallocate(c, 64, 4);
      SECTION("recovered memory")
      {
        auto d = m.allocate(64, 4);
        REQUIRE(d != nullptr);
        REQUIRE(d == c);
        m.deallocate(d, 64, 4);
      }
    }
    m.deallocate(b, 64, 4);
    m.deallocate(a, 64, 4);
    SECTION("recovered memory")
    {
      auto c = m.allocate(64, 4);
      REQUIRE(c != nullptr);
      REQUIRE(a == c);
    }
  }
  SECTION("returns void")
  {
    fallback<monotonic<1, local<128, 4>>, local<128, 4>> m(
      std::piecewise_construct, std::forward_as_tuple(128, 4), std::forward_as_tuple());
    auto a = m.allocate(64, 4);
    REQUIRE(a != nullptr);
    REQUIRE(m.get_primary()[a] != nullptr);
    auto b = m.allocate(64, 4);
    REQUIRE(b != nullptr);
    REQUIRE(m.get_primary()[b] != nullptr);
    SECTION("Secondary")
    {
      auto c = m.allocate(64, 4);
      REQUIRE(c != nullptr);
      REQUIRE(m.get_primary()[c] == nullptr);
      m.deallocate(c, 64, 4);
      SECTION("recovered memory")
      {
        auto d = m.allocate(64, 4);
        REQUIRE(d != nullptr);
        REQUIRE(d == c);
        m.deallocate(d, 64, 4);
      }
    }
  }
}