#include "fence.h"

#include "free_block.h" // free_block
#include "stack.h" // stack

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  alignas(4) char buffer[128];
  fence<free_block<32, stack<4>>> m(buffer, 128, 4);
  REQUIRE(m.get_mem_block().first == buffer);
  REQUIRE(m.get_mem_block().last == (buffer + 128));
  auto a = m.allocate(64, 4);
  REQUIRE(a != nullptr);
  REQUIRE(a == buffer);
  REQUIRE(m.deallocate(a, 64, 4) == true);
  REQUIRE(m.deallocate(buffer + 128, 64, 4) == false);
  alignas(4) char not_owned_by_allocator;
  REQUIRE(m.deallocate(&not_owned_by_allocator, 1, 4) == false);
}