#include "local.h"

#include "free_block.h" // free_block
#include "stack.h" // stack

#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate/deallocate", "[modifiers]")
{
  local<128, 4, free_block<32, stack<4>>> m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(32, 4);
  REQUIRE(b != nullptr);
  auto c = m.allocate(64, 4);
  REQUIRE(c != nullptr);
  auto d = m.allocate(32, 4);
  REQUIRE(d == nullptr);
  m.deallocate(a, 32, 4);
  m.deallocate(b, 32, 4);
  m.deallocate(c, 64, 4);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<local<128, 4, free_block<32, stack<4>>>> == true);
}