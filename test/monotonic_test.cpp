#include "monotonic.h"

#include "free_block.h" // free_block
#include "local.h" // local
#include "stack.h" // stack
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  monotonic<free_block<32, 4, 1, stack<4>, local<128, 4>>> m;
  m.deallocate(nullptr, 128, 4);
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
  REQUIRE(is_resource_v<monotonic<free_block<32, 4, 1, stack<4>, local<128, 4>>>> == true);
}