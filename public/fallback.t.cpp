#include "fallback.h"

#include "free_block.h" // free_block
#include "heap.h" // heap
#include "local.h" // local
#include "stack.h" // stack

#include <catch.hpp>

#include <tuple> // forward_as_tuple

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  fallback<free_block<1, stack<4>, local<128, 4>>, heap> m(
    std::piecewise_construct, std::forward_as_tuple(32, 4), std::forward_as_tuple());
  SECTION("default constructor")
  {
    fallback<local<128, 4>, heap> n;
  }
  auto a = m.allocate(64, 4);
  REQUIRE(a != nullptr);
  REQUIRE(m.get_primary()[a] != nullptr);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  REQUIRE(m.get_primary()[b] != nullptr);

  // exhausted primary memeory
  auto c = m.allocate(64, 4);
  REQUIRE(c != nullptr);
  REQUIRE(m.get_primary()[c] == nullptr);
  m.get_fallback().deallocate(c, 64, 4);

  m.deallocate(a, 64, 4);
  auto const & n = m;
  n.get_primary();
  n.get_fallback();
}