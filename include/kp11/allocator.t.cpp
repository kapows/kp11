#include "allocator.h"

#include "heap.h" // heap

#include <catch.hpp>

#include <list> // list
#include <vector> // vector

using namespace kp11;

TEST_CASE("relation", "[relation]")
{
  heap m;
  std::vector<int, allocator<int, decltype(m)>> v(&m);
  REQUIRE(v.get_allocator() == v.get_allocator());
  heap n;
  std::list<int, allocator<int, decltype(n)>> l(&n);
  REQUIRE(v.get_allocator() != l.get_allocator());
}
TEST_CASE("basic allocation", "[basic]")
{
  heap m;
  std::vector<int, allocator<int, decltype(m)>> v(&m);
  REQUIRE(v.get_allocator().get_resource() == &m);
  v.push_back(5);
  v.push_back(10);
  v.push_back(15);
  REQUIRE(v[0] == 5);
  REQUIRE(v[1] == 10);
  REQUIRE(v[2] == 15);
}
TEST_CASE("rebinding", "[rebinding]")
{
  heap m;
  std::list<int, allocator<int, decltype(m)>> l(&m);
  l.push_back(5);
  l.push_back(10);
  l.push_back(15);
  REQUIRE(l.size() == 3);
  REQUIRE(l.front() == 5);
  REQUIRE(l.back() == 15);
}