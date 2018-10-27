#include "monotonic.h"

#include "heap.h" // heap
#include "traits.h" // is_owner_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("max_size", "[max_size]")
{
  REQUIRE(monotonic<128, 4, 2, heap>::max_size() == 128);
  REQUIRE(monotonic<256, 4, 2, heap>::max_size() == 256);
}
TEST_CASE("constructor", "[constructor]")
{
  monotonic<128, 4, 2, heap> m;
  SECTION("move")
  {
    auto n = std::move(m);
  }
  SECTION("move assignment")
  {
    decltype(m) n;
    n = std::move(m);
  }
}
TEST_CASE("accessor", "[accessor]")
{
  monotonic<128, 4, 2, heap> m;
  [[maybe_unused]] auto & a = m.get_upstream();
  auto const & n = m;
  [[maybe_unused]] auto & b = n.get_upstream();
}
TEST_CASE("allocate", "[allocate]")
{
  monotonic<128, 4, 2, heap> m;
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
  monotonic<128, 4, 2, heap> m;
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
  monotonic<128, 4, 2, heap> m;
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
  REQUIRE(is_owner_v<monotonic<128, 4, 2, heap>> == true);
}