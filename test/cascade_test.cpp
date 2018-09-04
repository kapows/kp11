#include "cascade.h"

#include "free_block.h" // free_block
#include "heap.h" // heap
#include "stack.h" // stack

#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate/deallocate", "[modifiers]")
{
  cascade<128, 4, free_block<32, stack<4>>, heap> m;

  auto a = m.allocate(128, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(128, 4);
  REQUIRE(b != nullptr);
  REQUIRE(a != b);
  m.deallocate(b, 128, 4);
  m.deallocate(a, 128, 4);
}
TEST_CASE("find", "[element_access]")
{
  cascade<128, 4, free_block<32, stack<4>>, heap> m;
  auto const & cm = m;
  auto a = m.allocate(128, 4);
  auto it = m.find(a);
  REQUIRE(a == it.first.first);
  auto b = m.allocate(64, 4);
  auto it2 = m.find(b);
  REQUIRE(b == it2.first.first);
  REQUIRE(b == cm.find(b).first.first);
  auto c = it2.second.allocate(64, 4);
  REQUIRE(m.find(c).first == m.find(b).first);

  m.deallocate(b, 128, 4);
  m.deallocate(a, 128, 4);
}