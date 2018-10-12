#include "free_block.h"

#include "heap.h" // heap
#include "stack.h" // stack
#include "traits.h" // is_owner_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("constructor", "[constructor]")
{
  free_block<2, stack<4>, heap> m(128, 4);
  REQUIRE(m.get_block_size() == 32);
  SECTION("initial allocation")
  {
    decltype(m) n(128, 4, 2);
  }
  SECTION("move")
  {
    auto n = std::move(m);
    REQUIRE(n.get_block_size() == 32);
  }
  SECTION("move assignment")
  {
    free_block<2, stack<4>, heap> n(32, 4);
    REQUIRE(n.get_block_size() == 8);
    n = std::move(m);
    REQUIRE(n.get_block_size() == 32);
  }
}
TEST_CASE("accessor", "[accessor]")
{
  free_block<2, stack<4>, heap> m(128, 4);
  [[maybe_unused]] auto & a = m.get_upstream();
  auto const & n = m;
  [[maybe_unused]] auto & b = n.get_upstream();
}
TEST_CASE("operator[]", "[operator[]]")
{
  free_block<2, stack<4>, heap> m(128, 4);
  SECTION("failure")
  {
    REQUIRE(m[&m] == nullptr);
  }
  SECTION("success")
  {
    auto a = m.allocate(128, 4);
    REQUIRE(m[a] != nullptr);
  }
}
TEST_CASE("allocate", "[allocate]")
{
  free_block<2, stack<4>, heap> m(128, 4);
  auto a = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  SECTION("allocate a new memory block")
  {
    auto b = m.allocate(128, 4);
    REQUIRE(b != nullptr);
    REQUIRE(m[a] != m[b]);
    SECTION("failure")
    {
      auto c = m.allocate(128, 4);
      REQUIRE(c == nullptr);
    }
  }
}
TEST_CASE("deallocate", "[deallocate]")
{
  free_block<2, stack<4>, heap> m(128, 4);
  auto a = m.allocate(128, 4);
  auto b = m.allocate(128, 4);
  SECTION("success")
  {
    REQUIRE(m.deallocate(a, 128, 4) == true);
    REQUIRE(m.deallocate(b, 128, 4) == true);
  }
  SECTION("failure")
  {
    SECTION("doesn't belong to us")
    {
      REQUIRE(m.deallocate(&m, sizeof(m), alignof(decltype(m))) == false);
    }
    SECTION("nullptr")
    {
      auto c = m.allocate(128, 4);
      REQUIRE(c == nullptr);
      REQUIRE(m.deallocate(c, 128, 4) == false);
    }
  }
}
TEST_CASE("release", "[release]")
{
  free_block<2, stack<4>, heap> m(128, 4);
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
  REQUIRE(is_owner_v<free_block<1, stack<4>, heap>> == true);
}