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
    minimal_test_resource y;
    auto const & z = y;
    [[maybe_unused]] Resource k = z;
    Resource x = y;
    y = x;
    REQUIRE(std::is_same_v<decltype(x)::pointer, void *>);
    REQUIRE(std::is_same_v<decltype(x)::size_type,
      std::make_unsigned_t<typename std::pointer_traits<void *>::difference_type>>);
    REQUIRE(decltype(x)::max_size() == std::numeric_limits<std::size_t>::max());
    REQUIRE(x.allocate(static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == nullptr);
    x.deallocate(nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4));
  }
  SECTION("full")
  {
    Resource<test_resource> x;
    REQUIRE(std::is_same_v<decltype(x)::pointer, void *>);
    REQUIRE(std::is_same_v<decltype(x)::size_type,
      std::make_unsigned_t<typename std::pointer_traits<void *>::difference_type>>);
    REQUIRE(decltype(x)::max_size() == test_resource::max_size());
    REQUIRE(x.allocate(static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == nullptr);
    x.deallocate(nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4));
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
    Owner<minimal_test_owner> x;
    REQUIRE(x[nullptr] == nullptr);
    REQUIRE(
      x.deallocate(nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == false);
  }
  SECTION("full")
  {
    Owner<test_owner> x;
    REQUIRE(x[nullptr] == nullptr);
    REQUIRE(
      x.deallocate(nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == false);
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
    Marker<minimal_test_marker> m;
    [[maybe_unused]] minimal_test_marker n = m;
    REQUIRE(m.size() == 10);
    REQUIRE(m.count() == 0);
    REQUIRE(m.max_size() == minimal_test_marker::size());
    REQUIRE(m.max_alloc() == 10);
    REQUIRE(m.allocate(10) == 0);
    m.deallocate(0, 10);
  }
  SECTION("full")
  {
    Marker<test_marker> m;
    [[maybe_unused]] test_marker n = m;
    REQUIRE(m.size() == 10);
    REQUIRE(m.count() == 0);
    REQUIRE(m.max_size() == test_marker::size());
    REQUIRE(m.max_alloc() == 10);
    REQUIRE(m.allocate(10) == 0);
    m.deallocate(0, 10);
  }
}
TEST_CASE("is_marker", "[marker_traits]")
{
  REQUIRE(is_marker_v<int> == false);
  REQUIRE(is_marker_v<test_marker> == true);
  REQUIRE(is_marker_v<minimal_test_marker> == true);
}