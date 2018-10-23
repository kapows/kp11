#include "segregator.h"

#include "free_block.h" // free_block
#include "local.h" // local
#include "stack.h" // stack

#include <catch.hpp>

using namespace kp11;

using small_t = free_block<128, 4, 1, stack<4>, local<128, 4>>;
using large_t = free_block<256, 4, 1, stack<4>, local<256, 4>>;

TEST_CASE("constructor", "[constructor]")
{
  SECTION("default")
  {
    segregator<local<128, 4>, local<256, 4>> m(128);
  }
}
TEST_CASE("accessor", "[accessor]")
{
  segregator<local<128, 4>, local<256, 4>> m(128);
  [[maybe_unused]] auto & a = m.get_small();
  [[maybe_unused]] auto & b = m.get_large();
  auto const & n = m;
  [[maybe_unused]] auto & c = n.get_small();
  [[maybe_unused]] auto & d = n.get_large();
}
TEST_CASE("allocate", "[allocate]")
{
  segregator<small_t, large_t> m(128);
  SECTION("small")
  {
    auto a = m.allocate(64, 4);
    REQUIRE(a != nullptr);
    REQUIRE(m.get_small()[a] != nullptr);
    auto b = m.allocate(64, 4);
    REQUIRE(b != nullptr);
    REQUIRE(b != a);
    REQUIRE(m.get_small()[a] == m.get_small()[b]);
  }
  SECTION("large")
  {
    auto a = m.allocate(128, 4);
    REQUIRE(a != nullptr);
    REQUIRE(m.get_large()[a] != nullptr);
    auto b = m.allocate(128, 4);
    REQUIRE(b != nullptr);
    REQUIRE(b != a);
    REQUIRE(m.get_large()[a] == m.get_large()[b]);
  }
}
TEST_CASE("deallocate", "[deallocate]")
{
  segregator<small_t, large_t> m(128);
  SECTION("small")
  {
    auto a = m.allocate(64, 4);
    SECTION("recovers memory")
    {
      m.deallocate(a, 64, 4);
      auto b = m.allocate(64, 4);
      REQUIRE(b != nullptr);
      REQUIRE(b == a);
    }
  }
  SECTION("large")
  {
    auto a = m.allocate(128, 4);
    REQUIRE(a != nullptr);
    SECTION("recovers memory")
    {
      m.deallocate(a, 128, 4);
      auto b = m.allocate(128, 4);
      REQUIRE(b != nullptr);
      REQUIRE(b == a);
    }
  }
}
