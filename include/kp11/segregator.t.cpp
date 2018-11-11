#include "segregator.h"

#include "free_block.h" // free_block
#include "heap.h" // heap
#include "local.h" // local
#include "stack.h" // stack

#include <catch.hpp>

using namespace kp11;

using small_t = free_block<128, 4, 1, stack<4>, local<128, 4>>; // 32 byte blocks
using large_t = free_block<256, 4, 1, stack<4>, local<256, 4>>; // 64 byte blocks
using non_owner_large_t = heap;

TEST_CASE("max_size", "[max_size]")
{
  REQUIRE(segregator<128, small_t, large_t>::max_size() == large_t::max_size());
}
TEST_CASE("accessor", "[accessor]")
{
  segregator<128, small_t, large_t> m;
  [[maybe_unused]] auto & a = m.get_small();
  [[maybe_unused]] auto & b = m.get_large();
}
TEST_CASE("allocate", "[allocate]")
{
  segregator<128, small_t, large_t> m;
  SECTION("large is an owner")
  {
    auto a = m.allocate(64, 4); // small
    REQUIRE(a != nullptr);
    REQUIRE(m.get_small()[a] != nullptr);
    REQUIRE(m[a] != nullptr);
    auto b = m.allocate(160, 4); // large
    REQUIRE(b != nullptr);
    REQUIRE(m.get_small()[b] == nullptr);
    REQUIRE(m.get_large()[b] != nullptr);
    REQUIRE(m[b] != nullptr);
  }
  SECTION("large is not an owner")
  {
    auto a = m.allocate(64, 4); // small
    REQUIRE(a != nullptr);
    REQUIRE(m.get_small()[a] != nullptr);
    auto b = m.allocate(160, 4); // large
    REQUIRE(b != nullptr);
    REQUIRE(m.get_small()[b] == nullptr);
    REQUIRE(m.get_large()[b] != nullptr);
    m.deallocate(b, 160, 4);
  }
}
TEST_CASE("deallocate", "[deallocate]")
{
  segregator<128, small_t, large_t> m;
  SECTION("large is an owner")
  {
    auto a = m.allocate(64, 4); // small
    auto b = m.allocate(160, 4); // large
    REQUIRE(m.deallocate(a, 64, 4) == true);
    REQUIRE(m.deallocate(b, 160, 4) == true);
    REQUIRE(m.deallocate(&m, 160, 4) == false);
  }
  SECTION("large is not an owner")
  {
    [[maybe_unused]] auto a = m.allocate(64, 4); // small
    auto b = m.allocate(160, 4); // large
    m.deallocate(a, 64, 4);
    m.deallocate(b, 160, 4);
  }
}
