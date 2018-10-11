#include "monotonic.h"

#include "heap.h" // heap
#include "traits.h" // is_owner_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("constructor", "[constructor]")
{
  monotonic<2, heap> m(128, 4);
  SECTION("initial allocation")
  {
    decltype(m) n(128, 4, true);
  }
  SECTION("move")
  {
    auto n = std::move(m);
  }
  SECTION("move assignment")
  {
    decltype(m) n(32, 4);
    n = std::move(m);
  }
}
TEST_CASE("allocate", "[allocate]")
{
  monotonic<2, heap> m(128, 4);
  auto a = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  SECTION("new memory block")
  {
    auto b = m.allocate(128, 4);
    REQUIRE(b != nullptr);
    REQUIRE(b != a);
    SECTION("failure")
    {
      auto c = m.allocate(128, 4);
      REQUIRE(c == nullptr);
    }
  }
}
TEST_CASE("operator[]", "[operator[]]")
{
  monotonic<2, heap> m(128, 4);
  SECTION("success")
  {
    auto a = m.allocate(128, 4);
    REQUIRE(m[a] != nullptr);
  }
  SECTION("failure")
  {
    REQUIRE(m[&m] == nullptr);
  }
}
TEST_CASE("release", "[release]")
{
  monotonic<2, heap> m(128, 4);
  auto a = m.allocate(128, 4);
  auto b = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  m.release();
  auto c = m.allocate(128, 4);
  REQUIRE(c != nullptr);
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_owner_v<monotonic<2, heap>> == true);
}