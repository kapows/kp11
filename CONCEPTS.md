# Concepts

Unless otherwise stated, expressions do not throw exceptions.

## Resource

The `Resource` concept describes types that can allocate and deallocate memory.

### Requirements

The type `R` satisfies `Resource` if

Given:
* `r` is an lvalue of type `R`
* `ptr` a pointer of type `R::pointer`
* `size` a value of type `R::size_type`
* `alignment` a value of type `R::size_type`

The following expressions must be valid and meet their specified requirements:

| Expression | Requirements | Return Type |
| ---------- | ------------ | ----------- | 
| `R::pointer` |  Satisfies `NullablePointer` and `RandomAccessIterator` | | 
| `R::size_type` | Can represent the size of the largest object `r` can allocate. | |
| `R r` | The object `r` is default-constructible | |
| `ptr = r.allocate(size, alignment)` | Unless `ptr` is `nullptr` it is not returned again unless it has been passed to `r.deallocate(ptr, size, alignment)`. | `R::pointer` |
| `r.deallocate(ptr, size, alignment)` | | unspecified |

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
* `r` is an lvalue of type `R`
* `ptr` a pointer of type `R::pointer`
* `size` a value of type `R::size_type`
* `alignment` a value of type `R::size_type`
* `b` a value of type `bool`

The following expressions must be valid and meet their specified requirements:

| Expression | Requirements | Return Type |
| ---------- | ------------ | ----------- | 
| `ptr = r[ptr]` | | `R::pointer` |
| `b = r.deallocate(ptr, size, alignment)` or `r.deallocate(ptr, size, alignment)` | | convertible to `bool`, otherwise unspecified |

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
* `r` is an lvalue of type `R`
* `i` a value of type `R::size_type`
* `n` a value of type `R::size_type`

The following expressions must be valid and meet their specified requirements:

| Expression | Requirements | Return Type |
| ---------- | ------------ | ----------- | 
| `R::size_type` | Can represent the maximum number of indexes `r` can allocate. | | 
| `n = R::max_size()` | | `R::size_type` | 
| `n = r.size()` | `n <= R::max_size()` | `R::size_type` | 
| `n = r.max_alloc()` | `n <= R::max_size() - r.size()` | `R::size_type` | 
| `i = r.allocate(n)` | `n <= r.max_alloc()`. `i < R::max_size()`. `[i,i+n)` is not returned until a call to `r.deallocate(i, n)`. | `R::size_type` | 
| `r.deallocate(i, n)` | | unused | 

### Exemplar

```cpp
class marker
{
public:
  using size_type = std::size_t;
  static constexpr size_type max_size() noexcept;
  size_type size() const noexcept;
  size_type max_alloc() const noexcept;
  size_type allocate(size_type n) noexcept;
  void deallocate(size_type index, size_type n) noexcept;
};
```