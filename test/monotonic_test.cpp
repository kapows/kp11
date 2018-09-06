#include "monotonic.h"

#include "free_block.h" // free_block
#include "stack.h" // stack
#include "traits.h" // is_resource_v, is_strategy_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  alignas(4) char buffer[128];
  monotonic<free_block<32, stack<4>>> m(buffer, 128, 4);
  m.deallocate(buffer, 128, 4);
  m.deallocate(nullptr, 128, 4);
  m.deallocate(buffer + 10, 128, 4);
  auto a = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  SECTION("deallocate is a no-op")
  {
    m.deallocate(a, 128, 4);
    REQUIRE(m.allocate(128, 4) == nullptr);
  }
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_strategy_v<monotonic<free_block<32, stack<4>>>> == true);
}