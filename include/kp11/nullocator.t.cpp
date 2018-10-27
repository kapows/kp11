#include "nullocator.h"

#include "traits.h" // is_resource_v

#include <catch.hpp>

#include <limits> // numeric_limits

using namespace kp11;

TEST_CASE("max_size", "[max_size]")
{
  REQUIRE(nullocator::max_size() == std::numeric_limits<typename nullocator::size_type>::max());
}
TEST_CASE("allocate/deallocate", "[allocate/deallocate]")
{
  nullocator m;
  auto a = m.allocate(32, 4);
  REQUIRE(a == nullptr);
  auto b = m.allocate(64, 8);
  REQUIRE(b == nullptr);
  m.deallocate(a, 32, 4);
  m.deallocate(b, 64, 8);
}
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_resource_v<nullocator> == true);
}