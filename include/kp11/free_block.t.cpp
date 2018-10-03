#include "free_block.h"

#include "heap.h" // heap
#include "stack.h" // stack
#include "traits.h" // is_resource_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  free_block<2, stack<4>, heap> m(32, 4);
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(64, 4);
  REQUIRE(b != nullptr);
  REQUIRE(a != b);
  REQUIRE(m[a] == m[b]);
  auto c = m.allocate(32, 4);
  REQUIRE(c != nullptr);
  REQUIRE(b != c);
  REQUIRE(m[b] == m[c]);
  // replicate
  auto d = m.allocate(32, 4);
  REQUIRE(d != nullptr);
  REQUIRE(m[a] != m[d]);
  SECTION("deallocate recovers with stack functionality")
  {
    REQUIRE(m.deallocate(c, 32, 4) == true);
    REQUIRE(m.deallocate(b, 64, 4) == true);
    REQUIRE(m.deallocate(a, 32, 4) == true);

    auto e = m.allocate(32, 4);
    REQUIRE(e != nullptr);
    REQUIRE(e == a);
    auto f = m.allocate(96, 4);
    REQUIRE(f != nullptr);
    REQUIRE(f == b);
  }
  SECTION("exhausted memory and replicas")
  {
    auto e = m.allocate(128, 4);
    REQUIRE(e == nullptr);
    REQUIRE(m.deallocate(e, 128, 4) == false);
  }

  REQUIRE(m.deallocate(d, 32, 4) == true);
  REQUIRE(m.deallocate(c, 32, 4) == true);
  REQUIRE(m.deallocate(b, 64, 4) == true);
  REQUIRE(m.deallocate(a, 32, 4) == true);

  SECTION("upstream accessors")
  {
    auto const & n = m;
    auto & u = m.get_upstream();
    auto & v = n.get_upstream();
    auto p = u.allocate(32, 4);
    u.deallocate(p, 32, 4);
    REQUIRE(&v == &u);
  }
  m.release();
}

TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<free_block<1, stack<4>, heap>> == true);
  REQUIRE(is_owner_v<free_block<1, stack<4>, heap>> == true);
}