#include "memory_resource.h"

#include "heap.h" // heap

#include <catch.hpp>

#include <memory_resource> // pmr::polymorphic_allocator
#include <vector> // vector

using namespace kp11;

TEST_CASE("unit test", "[unit-test]")
{
  memory_resource<heap> m;
  std::vector<int, std::pmr::polymorphic_allocator<int>> v(&m);
  v.push_back(5);
  v.push_back(10);
  v.push_back(15);

  REQUIRE(v[0] == 5);
  REQUIRE(v[1] == 10);
  REQUIRE(v[2] == 15);
}
