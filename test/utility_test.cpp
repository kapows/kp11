#include "utility.h"

#include <catch.hpp>

#include <cstddef> // size_t

using namespace kp11;

TEST_CASE("advance unit test", "[unit-test]")
{
  char buffer[128];
  void * ptr = buffer;
  void * end = buffer + 128;
  REQUIRE(advance(ptr, 128) == end);
}
TEST_CASE("mem_block unit test", "[unit-test]")
{
  char buffer[128];
  SECTION("construct with two pointers")
  {
    mem_block<void *, std::size_t> blk(buffer, buffer + 128);
    REQUIRE(blk.first == buffer);
    REQUIRE(blk.last == (buffer + 128));
  }
  SECTION("construct with pointer and size")
  {
    mem_block<void *, std::size_t> blk(buffer, 128);
    REQUIRE(blk.first == buffer);
    REQUIRE(blk.last == (buffer + 128));
  }
  mem_block<void *, std::size_t> blk(buffer, 128);
  SECTION("comparison")
  {
    decltype(blk) blk2(buffer, 64);
    decltype(blk) blk3(buffer + 64, 64);
    REQUIRE((blk2 < blk2) == false);
    REQUIRE((blk2 < blk3) == true);
    REQUIRE(blk2 == blk2);
    REQUIRE(blk2 != blk3);
    REQUIRE(blk != blk2);
    REQUIRE(blk == blk);
  }
  SECTION("contains")
  {
    REQUIRE(blk.contains(buffer) == true);
    REQUIRE(blk.contains(buffer + 10) == true);
    REQUIRE(blk.contains(buffer + 127) == true);
    REQUIRE(blk.contains(buffer + 128) == false);
  }
}