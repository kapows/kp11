#include "heap.h"
#include "traits.h"

#include <catch.hpp>

#include <string>

using namespace kp11;

TEST_CASE("is_resource", "[modifiers]")
{
  REQUIRE(is_resource_v<int> == false);
  REQUIRE(is_resource_v<float> == false);
  REQUIRE(is_resource_v<std::string> == false);
  REQUIRE(is_resource_v<heap> == true);
}