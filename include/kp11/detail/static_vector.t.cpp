#include "static_vector.h"

#include <catch.hpp>

using namespace kp11::detail;

TEST_CASE("unit test", "[unit-test]")
{
  static_vector<int, 10> xs;
  REQUIRE(xs.capacity() == 10);
  REQUIRE(xs.max_size() == 10);
  REQUIRE(xs.size() == 0);
  REQUIRE(xs.empty() == true);
  xs.push_back(5);
  REQUIRE(xs.size() == 1);
  REQUIRE(xs[0] == 5);
  REQUIRE(xs.empty() == false);
  int i = 10;
  xs.push_back(i);
  REQUIRE(xs.size() == 2);
  REQUIRE(xs[1] == 10);
  REQUIRE(xs.empty() == false);

  xs.emplace_back(15);
  REQUIRE(xs.size() == 3);
  REQUIRE(xs[2] == 15);
  REQUIRE(xs[1] == 10);
  REQUIRE(xs[0] == 5);
  REQUIRE(xs.empty() == false);
  REQUIRE(xs.front() == 5);
  REQUIRE(xs.back() == 15);

  SECTION("const element access")
  {
    auto const & ys = xs;
    REQUIRE(ys[0] == 5);
    REQUIRE(ys[1] == 10);
    REQUIRE(ys[2] == 15);
    REQUIRE(ys.front() == 5);
    REQUIRE(ys.back() == 15);
  }
  SECTION("copy constructor")
  {
    auto ys = xs;
    REQUIRE(ys == xs);
    ys.push_back(20);
    REQUIRE(ys != xs);
  }
  SECTION("copy assignment")
  {
    decltype(xs) ys;
    ys = xs;
    REQUIRE(ys == xs);
    ys.push_back(20);
    REQUIRE(ys != xs);
    REQUIRE(ys.size() == 4);
  }
  SECTION("non const iterators")
  {
    auto first = xs.begin();
    auto last = xs.end();
    REQUIRE(last - first == 3);
    REQUIRE(first != last);
    REQUIRE(*first == 5);
    ++first;
    REQUIRE(*first == 10);
    ++first;
    REQUIRE(*first == 15);
    ++first;
    REQUIRE(first == last);
  }
  SECTION("const iterators")
  {
    auto const & ys = xs;
    auto first = ys.begin();
    REQUIRE(first == xs.cbegin());
    auto last = ys.end();
    REQUIRE(last == xs.cend());
    REQUIRE(last - first == 3);
    REQUIRE(first != last);
    REQUIRE(*first == 5);
    ++first;
    REQUIRE(*first == 10);
    ++first;
    REQUIRE(*first == 15);
    ++first;
    REQUIRE(first == last);
  }
  SECTION("pop_back")
  {
    xs.pop_back();
    REQUIRE(xs.size() == 2);
    REQUIRE(xs[1] == 10);
    REQUIRE(xs[0] == 5);
    REQUIRE(xs.empty() == false);

    xs.pop_back();
    REQUIRE(xs.size() == 1);
    REQUIRE(xs[0] == 5);
    REQUIRE(xs.empty() == false);

    xs.pop_back();
    REQUIRE(xs.size() == 0);
    REQUIRE(xs.empty() == true);
  }
  SECTION("clear")
  {
    xs.clear();
    REQUIRE(xs.size() == 0);
    REQUIRE(xs.empty() == true);
  }
}