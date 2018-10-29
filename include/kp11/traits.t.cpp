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
  static constexpr size_type max_size() noexcept
  {
    return 12;
  }
  pointer allocate(size_type size, size_type alignment) noexcept
  {
    return nullptr;
  }
  void deallocate(pointer ptr, size_type size, size_type alignment) noexcept
  {
  }
};
/// @private
class minimal_test_resource
{
public:
  using pointer = void *;
  void * allocate(std::size_t size, std::size_t alignment) noexcept
  {
    return nullptr;
  }
  void deallocate(void * ptr, std::size_t size, std::size_t alignment) noexcept
  {
  }
};

TEST_CASE("resource_traits", "[resource_traits]")
{
  SECTION("minimal")
  {
    minimal_test_resource x;
    using rt = resource_traits<minimal_test_resource>;
    REQUIRE(std::is_same_v<rt::pointer, void *>);
    REQUIRE(std::is_same_v<rt::size_type,
      std::make_unsigned_t<typename std::pointer_traits<void *>::difference_type>>);
    REQUIRE(rt::max_size() == std::numeric_limits<std::size_t>::max());
    REQUIRE(rt::allocate(x, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == nullptr);
    rt::deallocate(x, nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4));
  }
  SECTION("full")
  {
    test_resource x;
    using rt = resource_traits<test_resource>;
    REQUIRE(rt::max_size() == test_resource::max_size());
    REQUIRE(rt::allocate(x, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == nullptr);
    rt::deallocate(x, nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4));
  }
}
TEST_CASE("is_resource", "[resource_traits]")
{
  REQUIRE(is_resource_v<int> == false);
  REQUIRE(is_resource_v<float> == false);
  REQUIRE(is_resource_v<test_resource> == true);
  REQUIRE(is_resource_v<minimal_test_resource> == true);
}

/// @private
class test_owner : public test_resource
{
public:
  bool deallocate(pointer ptr, size_type size, size_type alignment) noexcept
  {
    return false;
  }
  pointer operator[](pointer ptr) const noexcept
  {
    return nullptr;
  }
};
/// @private
class minimal_test_owner : public minimal_test_resource
{
public:
  void * operator[](void * ptr) const noexcept
  {
    return nullptr;
  }
};
TEST_CASE("owner_traits", "[owner_traits]")
{
  SECTION("minimal")
  {
    minimal_test_owner x;
    using ot = owner_traits<minimal_test_owner>;
    REQUIRE(ot::deallocate(x, nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) ==
            false);
  }
  SECTION("full")
  {
    test_owner x;
    using ot = owner_traits<test_owner>;
    REQUIRE(ot::deallocate(x, nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) ==
            false);
  }
}
TEST_CASE("is_owner", "[owner_traits]")
{
  REQUIRE(is_owner_v<int> == false);
  REQUIRE(is_owner_v<test_owner> == true);
  REQUIRE(is_owner_v<minimal_test_owner> == true);
}

/// @private
class test_marker
{
public:
  using size_type = std::size_t;
  static constexpr size_type size() noexcept
  {
    return 10;
  }
  size_type count() const noexcept
  {
    return 0;
  }
  static constexpr size_type max_size() noexcept
  {
    return size();
  }
  size_type max_alloc() const noexcept
  {
    return 10;
  }
  size_type allocate(size_type n) noexcept
  {
    return 0;
  }
  void deallocate(size_type index, size_type n) noexcept
  {
  }
};

/// @private
class minimal_test_marker
{
public:
  using size_type = std::size_t;
  static constexpr size_type size() noexcept
  {
    return 10;
  }
  size_type count() const noexcept
  {
    return 0;
  }
  size_type max_alloc() const noexcept
  {
    return 10;
  }
  size_type allocate(size_type n) noexcept
  {
    return 0;
  }
  void deallocate(size_type index, size_type n) noexcept
  {
  }
};

TEST_CASE("marker_traits", "[marker_traits]")
{
  SECTION("minimal")
  {
    using mt = marker_traits<minimal_test_marker>;
    minimal_test_marker m;
    REQUIRE(mt::size() == 10);
    REQUIRE(mt::count(m) == 0);
    REQUIRE(mt::max_size() == minimal_test_marker::size());
    REQUIRE(mt::max_alloc(m) == 10);
    REQUIRE(mt::allocate(m, 10) == 0);
    mt::deallocate(m, 0, 10);
  }
  SECTION("full")
  {
    using mt = marker_traits<test_marker>;
    test_marker m;
    REQUIRE(mt::size() == 10);
    REQUIRE(mt::count(m) == 0);
    REQUIRE(mt::max_size() == test_marker::size());
    REQUIRE(mt::max_alloc(m) == 10);
    REQUIRE(mt::allocate(m, 10) == 0);
    mt::deallocate(m, 0, 10);
  }
}
TEST_CASE("is_marker", "[marker_traits]")
{
  REQUIRE(is_marker_v<int> == false);
  REQUIRE(is_marker_v<test_marker> == true);
  REQUIRE(is_marker_v<minimal_test_marker> == true);
}