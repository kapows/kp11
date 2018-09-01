#include "monotonic.h"

#include "free_block.h" // free_block
#include "local.h" // local
#include "stack.h" // stack
#include "traits.h" // is_strategy_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate/deallocate", "[modifiers]")
{
  local<128, 4, monotonic<free_block<32, stack<4>>>> m;
  auto a = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  m.deallocate(a, 128, 4);
  // deallocate should be suppressed
  auto b = m.allocate(32, 4);
  REQUIRE(b == nullptr);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_strategy_v<local<128, 4, monotonic<free_block<32, stack<4>>>>> == true);
}