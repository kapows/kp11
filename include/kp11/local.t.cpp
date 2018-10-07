#include "local.h"

#include "traits.h" // is_owner_v

#include <catch.hpp>

using namespace kp11;

TEST_CASE("allocate", "[allocate]")
{
  local<128, 4> m;
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(32, 4);
  REQUIRE(b == nullptr);
}
TEST_CASE("deallocate", "[deallocate]")
{
  local<128, 4> m;
  auto a = m.allocate(32, 4);
  SECTION("success")
  {
    REQUIRE(m.deallocate(a, 32, 4) == true);
    SECTION("recovered memory")
    {
      auto b = m.allocate(32, 4);
      REQUIRE(b != nullptr);
      REQUIRE(b == a);
    }
  }
  SECTION("failure")
  {
    REQUIRE(m.deallocate(&m, 32, 4) == false);
  }
  SECTION("nullptr")
  {
    REQUIRE(m.deallocate(nullptr, 32, 4) == false);
  }
}
TEST_CASE("operator[]", "[operator[]]")
{
  local<128, 4> m;
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
TEST_CASE("traits", "[traits]")
{
  REQUIRE(is_owner_v<local<128, 4>> == true);
}