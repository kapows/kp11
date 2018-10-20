#include "traits.h"

#include <catch.hpp>

#include <cstddef> // size_t

using namespace kp11;

/// @private
class test_resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept;
  void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
};

/// @private
class test_not_a_resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept;
  void deallocate(size_type bytes, size_type alignment) noexcept;
};
TEST_CASE("is_resource", "[modifiers]")
{
  REQUIRE(is_resource_v<int> == false);
  REQUIRE(is_resource_v<float> == false);
  REQUIRE(is_resource_v<test_not_a_resource> == false);
  REQUIRE(is_resource_v<test_resource> == true);
}

/// @private
class test_owner
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept
  {
    return nullptr;
  }
  bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
  {
    return false;
  }
  pointer operator[](pointer ptr) const noexcept
  {
    return nullptr;
  }
};
/// @private
class another_test_owner
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept
  {
    return nullptr;
  }
  void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
  {
  }
  pointer operator[](pointer ptr) const noexcept
  {
    return nullptr;
  }
};
/// @private
class test_not_an_owner
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type bytes, size_type alignment) noexcept;
  bool deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept;
};
TEST_CASE("is_owner", "[traits]")
{
  REQUIRE(is_owner_v<test_owner> == true);
  REQUIRE(is_owner_v<test_not_an_owner> == false);
  REQUIRE(is_owner_v<int> == false);
}
TEST_CASE("owner_traits", "[traits]")
{
  SECTION("deallocate returns bool")
  {
    test_owner m;
    auto ptr = m.allocate(32, 4);
    owner_traits<test_owner>::deallocate(m, ptr, 32, 4);
  }
  SECTION("deallocate does not return bool")
  {
    another_test_owner m;
    auto ptr = m.allocate(32, 4);
    owner_traits<another_test_owner>::deallocate(m, ptr, 32, 4);
  }
}

/// @private
template<std::size_t N>
class test_marker
{
public:
  using size_type = std::size_t;
  static constexpr size_type max_size() noexcept
  {
    return N;
  }
  size_type size() noexcept;
  size_type biggest() const noexcept;
  size_type set(size_type n) noexcept;
  void reset(size_type index, size_type n) noexcept;
};

/// @private
template<std::size_t N>
class test_not_a_marker
{
public:
  using size_type = std::size_t;
  size_type set(size_type n) noexcept;
  void reset(size_type index, size_type n) noexcept;
};
TEST_CASE("is_marker", "[traits]")
{
  REQUIRE(is_marker_v<int> == false);
  REQUIRE(is_marker_v<float> == false);
  REQUIRE(is_marker_v<test_not_a_marker<1>> == false);
  REQUIRE(is_marker_v<test_marker<1>> == true);
}