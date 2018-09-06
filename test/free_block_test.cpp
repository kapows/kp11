#include "free_block.h"

#include "stack.h"
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  alignas(4) char buffer[128];
  free_block<32, stack<4>> m(buffer, 128, 4);
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  auto c = m.allocate(32, 4);
  REQUIRE(c != nullptr);
  auto d = m.allocate(32, 4);
  REQUIRE(d == nullptr);

  SECTION("deallocate recovers with stack functionality")
  {
    m.deallocate(c, 32, 4);
    m.deallocate(b, 64, 4);
    m.deallocate(a, 32, 4);

    auto e = m.allocate(32, 4);
    REQUIRE(e != nullptr);
    REQUIRE(e == a);
    auto f = m.allocate(96, 4);
    REQUIRE(f != nullptr);
    REQUIRE(f == b);
  }
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<free_block<32, stack<4>>> == true);
}