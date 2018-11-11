#include "fallback.h"

#include "free_block.h" // free_block
#include "heap.h" // heap
#include "local.h" // local
#include "monotonic.h" // monotonic
#include "stack.h" // stack

#include <catch.hpp>

#include <tuple> // forward_as_tuple

using namespace kp11;

using primary_t = free_block<128, 4, 1, stack<4>, local<128, 4>>;
using secondary_t = local<128, 4>; // is an owner
using non_owner_secondary_t = heap;

TEST_CASE("max_size", "[max_size]")
{
  REQUIRE(fallback<primary_t, secondary_t>::max_size() == primary_t::max_size());
}
TEST_CASE("accessor", "[accessor]")
{
  fallback<primary_t, secondary_t> m;
  [[maybe_unused]] auto & a = m.get_primary();
  [[maybe_unused]] auto & b = m.get_secondary();
  auto const & n = m;
  [[maybe_unused]] auto & c = n.get_primary();
  [[maybe_unused]] auto & d = n.get_secondary();
}
TEST_CASE("allocate", "[allocate]")
{
  SECTION("secondary is an owner")
  {
    fallback<primary_t, secondary_t> m;
    auto a = m.allocate(64, 4);
    REQUIRE(a != nullptr);
    REQUIRE(m.get_primary()[a] != nullptr);
    REQUIRE(m[a] == m.get_primary()[a]);
    auto b = m.allocate(64, 4);
    REQUIRE(b != nullptr);
    REQUIRE(m.get_primary()[b] != nullptr);
    REQUIRE(m[b] == m.get_primary()[b]);
    SECTION("secondary")
    {
      auto c = m.allocate(64, 4);
      REQUIRE(c != nullptr);
      REQUIRE(m.get_primary()[c] == nullptr);
      REQUIRE(m.get_secondary()[c] != nullptr);
      REQUIRE(m[c] == m.get_secondary()[c]);
    }
  }
  SECTION("secondary is not an owner")
  {
    fallback<primary_t, non_owner_secondary_t> m;
    auto a = m.allocate(64, 4);
    REQUIRE(a != nullptr);
    REQUIRE(m.get_primary()[a] != nullptr);
    auto b = m.allocate(64, 4);
    REQUIRE(b != nullptr);
    REQUIRE(m.get_primary()[b] != nullptr);
    SECTION("secondary")
    {
      auto c = m.allocate(64, 4);
      REQUIRE(c != nullptr);
      REQUIRE(m.get_primary()[c] == nullptr);
      // we'll have to deallocate heap alloc
      m.deallocate(c, 64, 4);
    }
  }
}
TEST_CASE("deallocate", "[deallocate]")
{
  SECTION("secondary is an owner")
  {
    fallback<primary_t, secondary_t> m;
    auto a = m.allocate(64, 4); // primary
    auto b = m.allocate(64, 4); // primary
    auto c = m.allocate(64, 4); // secondary
    REQUIRE(m.deallocate(a, 64, 4) == true);
    REQUIRE(m.deallocate(b, 64, 4) == true);
    REQUIRE(m.deallocate(c, 64, 4) == true);
    REQUIRE(m.deallocate(&m, 64, 4) == false);
    SECTION("recovered memory")
    {
      auto d = m.allocate(64, 4);
      REQUIRE(d != nullptr);
      REQUIRE(m.get_primary()[d] != nullptr);
    }
  }
  SECTION("secondary is not an owner")
  {
    fallback<primary_t, non_owner_secondary_t> m;
    auto a = m.allocate(64, 4); // primary
    auto b = m.allocate(64, 4); // primary
    auto c = m.allocate(64, 4); // secondary
    m.deallocate(a, 64, 4);
    m.deallocate(b, 64, 4);
    m.deallocate(c, 64, 4);
    SECTION("recovered memory")
    {
      auto d = m.allocate(64, 4);
      REQUIRE(d != nullptr);
      REQUIRE(m.get_primary()[d] != nullptr);
    }
  }
}
TEST_CASE("operator[]", "[operator[]]")
{
  fallback<primary_t, secondary_t> m;
  SECTION("success")
  {
    auto a = m.allocate(32, 4);
    REQUIRE(m[a] != nullptr);
  }
  SECTION("failure")
  {
    REQUIRE(m[&m] == nullptr);
  }
}