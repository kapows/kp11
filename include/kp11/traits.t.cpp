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
    return 10;
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
  void * allocate(std::size_t size, std::size_t alignment) noexcept
  {
    return nullptr;
  }
  void deallocate(void * ptr, std::size_t size, std::size_t alignment) noexcept
  {
  }
};

/// @private
class test_not_a_resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type size, size_type alignment) noexcept;
  void deallocate(size_type size, size_type alignment) noexcept;
};
TEST_CASE("is_resource", "[modifiers]")
{
  REQUIRE(is_resource_v<int> == false);
  REQUIRE(is_resource_v<float> == false);
  REQUIRE(is_resource_v<test_not_a_resource> == false);
  REQUIRE(is_resource_v<test_resource> == true);
}
TEST_CASE("resource_traits", "[resource_traits]")
{
  SECTION("generated")
  {
    minimal_test_resource x;
    REQUIRE(std::is_same_v<resource_traits<minimal_test_resource>::pointer, void *>);
    REQUIRE(std::is_same_v<resource_traits<minimal_test_resource>::size_type,
      std::make_unsigned_t<typename std::pointer_traits<void *>::difference_type>>);
    REQUIRE(resource_traits<minimal_test_resource>::max_size() ==
            std::numeric_limits<std::size_t>::max());
    REQUIRE(resource_traits<minimal_test_resource>::allocate(
              x, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == nullptr);
    resource_traits<minimal_test_resource>::deallocate(
      x, nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4));
  }
  SECTION("explicit")
  {
    test_resource x;
    REQUIRE(resource_traits<test_resource>::max_size() == test_resource::max_size());
    REQUIRE(resource_traits<test_resource>::allocate(
              x, static_cast<std::size_t>(12), static_cast<std::size_t>(4)) == nullptr);
    resource_traits<test_resource>::deallocate(
      x, nullptr, static_cast<std::size_t>(12), static_cast<std::size_t>(4));
  }
}

/// @private
class test_owner
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  static constexpr size_type max_size() noexcept;
  pointer allocate(size_type size, size_type alignment) noexcept
  {
    return nullptr;
  }
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
class another_test_owner
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  static constexpr size_type max_size() noexcept;
  pointer allocate(size_type size, size_type alignment) noexcept
  {
    return nullptr;
  }
  void deallocate(pointer ptr, size_type size, size_type alignment) noexcept
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
  pointer allocate(size_type size, size_type alignment) noexcept;
  bool deallocate(pointer ptr, size_type size, size_type alignment) noexcept;
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
  static constexpr size_type size() noexcept
  {
    return N;
  }
  size_type count() noexcept
  {
    return size();
  }
  static constexpr size_type max_size() noexcept
  {
    return size();
  }
  size_type max_alloc() const noexcept
  {
    return size();
  }
  size_type allocate(size_type n) noexcept
  {
    return size();
  }
  void deallocate(size_type index, size_type n) noexcept
  {
  }
};

/// @private
template<std::size_t N>
class test_not_a_marker
{
public:
  using size_type = std::size_t;
  size_type allocate(size_type n) noexcept;
  void deallocate(size_type index, size_type n) noexcept;
};
TEST_CASE("is_marker", "[traits]")
{
  REQUIRE(is_marker_v<int> == false);
  REQUIRE(is_marker_v<float> == false);
  REQUIRE(is_marker_v<test_not_a_marker<1>> == false);
  REQUIRE(is_marker_v<test_marker<1>> == true);
}