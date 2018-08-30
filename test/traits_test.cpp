#include "traits.h"

#include "free_block.h"
#include "heap.h"
#include "stack.h"

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

TEST_CASE("is_marker", "[traits]")
{
  REQUIRE(is_marker_v<int> == false);
  REQUIRE(is_marker_v<float> == false);
  REQUIRE(is_marker_v<std::string> == false);
  REQUIRE(is_marker_v<heap> == false);
  REQUIRE(is_marker_v<stack<10>> == true);
}

TEST_CASE("is_strategy", "[traits]")
{
  REQUIRE(is_strategy_v<int> == false);
  REQUIRE(is_strategy_v<float> == false);
  REQUIRE(is_strategy_v<std::string> == false);
  REQUIRE(is_strategy_v<heap> == false);
  REQUIRE(is_strategy_v<stack<10>> == false);
  REQUIRE(is_strategy_v<free_block<32, 4, stack<10>>> == true);
}