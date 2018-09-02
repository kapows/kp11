#include "fence.h"

#include "free_block.h" // free_block
#include "stack.h" // stack
#include "traits.h" // is_strategy_v


#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate/deallocate", "[modifiers]")
{
  alignas(4) char buffer[128];
  fence<free_block<32, stack<4>>> m(buffer, 128, 4);

  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  REQUIRE(m.owns(a, 32, 4) == true);
  REQUIRE(m.deallocate(a, 32, 4) == true);

  alignas(4) char not_my_buffer[128];
  REQUIRE(m.owns(not_my_buffer, 32, 4) == false);
  REQUIRE(m.deallocate(not_my_buffer, 32, 4) == false);
}