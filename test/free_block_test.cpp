#include "free_block.h"

#include "stack.h"
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("constructor", "[constructors]")
{
  alignas(4) char buffer[128];
  free_block<32, stack<4>> m(buffer, 128, 4);
  (void)m;
}
TEST_CASE("allocate/deallocate", "[modifiers]")
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

  m.deallocate(c, 32, 4);
  m.deallocate(b, 64, 4);
  m.deallocate(a, 32, 4);

  d = m.allocate(128, 4);
  REQUIRE(d != nullptr);
  m.deallocate(d, 128, 4);
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<free_block<32, stack<4>>> == true);
}