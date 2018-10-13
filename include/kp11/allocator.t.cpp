#include "allocator.h"

#include "heap.h" // heap

#include "free_block.h"
#include "stack.h"

#include <catch.hpp>

#include <list> // list
#include <vector> // vector

using namespace kp11;

class resource
{
  free_block<2, stack<4>, heap> m;

public:
  using pointer = typename decltype(m)::pointer;
  using size_type = typename decltype(m)::size_type;
  resource() noexcept : m(256, 4)
  {
  }
  pointer allocate(size_type bytes, size_type alignment) noexcept
  {
    return m.allocate(bytes, alignment);
  }
  void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
  {
    m.deallocate(ptr, bytes, alignment);
  }
};
TEST_CASE("global relation", "[relation][global]")
{
  allocator<int, heap> x;
  REQUIRE(x == x);
  allocator<int, heap> y;
  REQUIRE(y == x);
  REQUIRE((y != x) == false);
  allocator<int, resource> z;
  REQUIRE(z == z);
}
TEST_CASE("global basic allocation", "[basic][global]")
{
  std::vector<int, allocator<int, heap>> v;
  v.push_back(5);
  v.push_back(10);
  v.push_back(15);
  REQUIRE(v[0] == 5);
  REQUIRE(v[1] == 10);
  REQUIRE(v[2] == 15);
}
TEST_CASE("global rebinding", "[rebinding][global]")
{
  std::list<int, allocator<int, heap>> l;
  l.push_back(5);
  l.push_back(10);
  l.push_back(15);
  REQUIRE(l.size() == 3);
  REQUIRE(l.front() == 5);
  REQUIRE(l.back() == 15);
}
TEST_CASE("local relation", "[relation][local]")
{
  heap m;
  std::vector<int, allocator<int, decltype(m) *>> v(&m);
  REQUIRE(v.get_allocator() == v.get_allocator());
  heap n;
  std::list<int, allocator<int, decltype(n) *>> l(&n);
  REQUIRE(v.get_allocator() != l.get_allocator());
}
TEST_CASE("local basic allocation", "[basic][local]")
{
  heap m;
  std::vector<int, allocator<int, decltype(m) *>> v(&m);
  REQUIRE(v.get_allocator().get_resource() == &m);
  v.push_back(5);
  v.push_back(10);
  v.push_back(15);
  REQUIRE(v[0] == 5);
  REQUIRE(v[1] == 10);
  REQUIRE(v[2] == 15);
}
TEST_CASE("local rebinding", "[rebinding][local]")
{
  heap m;
  std::list<int, allocator<int, decltype(m) *>> l(&m);
  l.push_back(5);
  l.push_back(10);
  l.push_back(15);
  REQUIRE(l.size() == 3);
  REQUIRE(l.front() == 5);
  REQUIRE(l.back() == 15);
}