#include "buffer.h"

#include "traits.h" // is_owner_v

#include <catch.hpp>

#include <limits> // numeric_limits

using namespace kp11;

TEST_CASE("max_size", "[max_size]")
{
  REQUIRE(buffer::max_size() == std::numeric_limits<typename buffer::size_type>::max());
}
TEST_CASE("allocate", "[allocate]")
{
  alignas(4) char buf[128];
  buffer m;
  m = {buf, 128, 4};
  auto a = m.allocate(32, 4);
  REQUIRE(a != nullptr);
  auto b = m.allocate(32, 4);
  REQUIRE(b == nullptr);
}
TEST_CASE("deallocate", "[deallocate]")
{
  alignas(4) char buf[128];
  buffer m(buf, 128, 4);
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
  alignas(4) char buf[128];
  buffer m(buf, 128, 4);
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
  REQUIRE(is_owner_v<buffer> == true);
}