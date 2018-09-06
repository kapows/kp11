#include "fallback.h"

#include "fence.h" // fence
#include "free_block.h" // free_block
#include "heap.h" // heap
#include "local.h" // local
#include "stack.h" // stack

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  fallback<local<128, 4, fence<free_block<32, stack<4>>>>, heap> m;
  auto a = m.allocate(64, 4);
  REQUIRE(a != nullptr);
  REQUIRE(m.get_primary().get_mem_block().contains(a) == true);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  REQUIRE(m.get_primary().get_mem_block().contains(b) == true);

  // exhausted primary memeory
  auto c = m.allocate(64, 4);
  REQUIRE(c != nullptr);
  REQUIRE(m.get_primary().get_mem_block().contains(c) == false);
  m.get_fallback().deallocate(c, 64, 4);

  auto const & n = m;
  n.get_primary();
  n.get_fallback();
}