# Concepts

## Resource

The `Resource` concept describes types that can allocate and deallocate memory.

### Requirements

The type `R` satisfies `Resource` if

Given:

* `u` is an identifier
* `r` is a value of type `R`
* `ptr` is a value of type `R::pointer`
* `size, alignment` are values of type `R::size_type`

The following types must be valid:

| Expression | Requirements | 
| ---------- | ------------ | 
| `R::pointer` |  Satisfies `NullablePointer` and `RandomAccessIterator` |
| `R::size_type` (optional) | Can represent the size of the largest object `r` can allocate. |

The following expressions must be valid:

| Expression | Return Type | Exception | Requirements | Semantics | 
| ---------- | ----------- | --------- | ------------ | --------- |
| `R u{}` | | `noexcept` | | |
| `R::max_size()` | `size_type` | `noexcept` | | Maximum size that can be passed to allocate. |
| `r.allocate(size, alignment)` | `pointer` | `noexcept` | `size <= R::max_size()`. Unless `ptr` is `nullptr` it is not returned again unless it has been passed to `r.deallocate(ptr, size, alignment)`. | Allocates memory suitable for `size` bytes, aligned to `alignment`. |
| `r.deallocate(ptr, size, alignment)` | | `noexcept` | | Deallocates memory allocated by `allocate`. |

### Exemplar

```cpp
class resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type size, size_type alignment) noexcept;
  void deallocate(pointer ptr, size_type size, size_type alignment) noexcept;
};
```

## Owner 

An `Owner` is a `Resource` with ownership semantics. 
Owners are able to tell whether or not memory was allocated by them.

### Requirements

The type `R` satisfies `Owner` if:

* `R` satisfies `Resource`

Given:

* `r` is a value of type `R`
* `ptr` is a value of type `R::pointer`
* `size, alignment` are values of type `R::size_type`
* `b` a value of type `bool`

The following expressions must be valid:

| Expression | Return Type | Exception | Requirements | Semantics |
| ---------- | ----------- | --------- | ------------ | --------- |
| `r[ptr]` | `pointer` | `noexcept` | | Returns pointer to the beginning of the memory that `ptr` points to if its owned by `r` otherwise `nullptr`. |
| `r.deallocate(ptr, size, alignment)` (optional) | convertible to `bool` | `noexcept` | | Returns `true` if `ptr` is owned by `r` and deallocates memory pointed to by `ptr` otherwise returns `false`. |

### Exemplar

```cpp
class owner
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  pointer allocate(size_type size, size_type alignment) noexcept;
  bool deallocate(pointer ptr, size_type size, size_type alignment) noexcept;
  pointer operator[](pointer ptr) const noexcept;
};
```

## Marker

The `Marker` concept describes types that can allocate and deallocate ranges of indexes.

### Requirements

The type `R` satisfies `Marker` if:

Given:

* `u` is an identifier
* `r` is a value of type `R`
* `i, n` are values of type `R::size_type`

The following types must be valid:

| Expression | Requirements | 
| ---------- | ------------ | 
| `R::size_type` | Can represent the maximum number of indexes `r` can allocate. |

The following expressions must be valid:

| Expression | Return Type | Exception | Requirements | Semantics |
| ---------- | ----------- | --------- | ------------ | --------- |
| `R u{}` | | `noexcept` | | |
| `R::size()` | `size_type` | `noexcept` | | Maximum amount of indexes that `R` can hold. |
| `r.count()` | `size_type` | `noexcept` | `r.count() <= R::size()` | Number of indexes that have been set. |
| `R::max_size()` (optional) | `size_type` | `noexcept` | `R::max_size() <= R::size()` | Maximum size that can be passed to `allocate`. |
| `r.allocate(n)` | `size_type` | `noexcept` | `n <= r.max_size()`. `r.allocate(n) <= R::size()`.  | Allocates `n` indexes. |
| `r.deallocate(i, n)` | | `noexcept` | `i` must have been returned by `allocate`. `n` must be the associated parameter used in the call to `allocate`. | Deallocates indexes `[i, i + n)`. |

### Exemplar

```cpp
class marker
{
public:
  using size_type = std::size_t;
  static constexpr size_type size() noexcept;
  size_type count() const noexcept;
  static constexpr size_type max_size() noexcept;
  size_type allocate(size_type n) noexcept;
  void deallocate(size_type i, size_type n) noexcept;
};
```