#include "heap.h"

#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate/deallocate", "[modifiers]")
{
  heap m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  m.deallocate(a, 32, 4);
}