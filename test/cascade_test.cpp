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
  auto a = m.allocate(128, 4);
  auto it = m.find(a);
  REQUIRE(it != m.end());
  auto b = m.allocate(128, 4);
  auto it2 = m.find(b);
  REQUIRE(it2 != m.end());
  alignas(4) char not_my_buffer[128];
  REQUIRE(m.find(not_my_buffer) == m.end());

  m.deallocate(b, 128, 4);
  m.deallocate(a, 128, 4);
}
TEST_CASE("iterators", "[iterators]")
{
  cascade<128, 4, free_block<32, stack<4>>, heap> m;
  auto const & cm = m;
  REQUIRE(m.begin() == m.end());
  REQUIRE(cm.begin() == cm.end());
  auto a = m.allocate(128, 4);
  REQUIRE(m.begin() != m.end());
  REQUIRE(m.cbegin() != m.cend());
  REQUIRE(cm.begin() != cm.end());
  m.deallocate(a, 128, 4);
}