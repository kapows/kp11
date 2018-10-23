# kp11

Easy to use, policy based memory resource builder.

## Introduction

C++17 has given us `<memory_resource>` and has basically shown us what stateful allocators should look like.

This library tries to make it easy to create memory resources and from those, make "good" allocators.
The general idea here is to allow the user to select a method of partitioning single blocks of memory into multiple blocks of memory to be allocated.
The library provides classes to allow the user to select different methods of partitioning by composing those classes. 
These classes are static and all overhead used in maintaining the internal data structure is internal to the class and is not stored in the allocated memory. 
Thus, there is a hard limit on the amount of allocations that can be made to an upstream resource. 
The allocated memory from the upstream is never touched by any of these classes.

## Inspiration

[CppCon 2015: Andrei Alexandrescu “std::allocator...”](https://www.youtube.com/watch?v=LIb3L4vKZ7U)

[CppCon 2017: Pablo Halpern “Allocators: The Good Parts”](https://www.youtube.com/watch?v=v3dz-AKOVL8)

[CppCon 2017: Bob Steagall “How to Write a Custom Allocator”](https://www.youtube.com/watch?v=kSWfushlvB8)

[C++Now 2018: Arthur O'Dwyer “An Allocator is a Handle to a Heap”](https://www.youtube.com/watch?v=0MdSJsCTRkY)

## Design

First and foremost the library is made to be easy to use. Since it is using a policy based design this basically means that everything is default constructed. If something more complicated is required then that will have to be done with an ugly two phase initialization style.

The library is broken up into a few pieces.
There is the upstream, the markers, the free blocks, control structures, and allocators.

Upstream refers to the `heap`, `local`, etc., style resources where the memory originates from.
Upstream resources are free to use their own "fancy pointers" and size type. This will be propagated downstream.
This should allow the control structures and free blocks to be used for any memory.

Markers refers to `list`, `pool`, etc., and are policies used in the `free_block` resource.

Free blocks refers to `free_block`, `monotonic`, etc., which break up allocated memory into blocks to allocate. 
All memory from free block resources including `monotonic` allocate in blocks.
In `monotonic` the block size is the alignment.
This is done so that no alignment needs to take place at all.

Control structures refer to `fallback`, `segregator`, etc., these are classes that combine other resources with a control structure.
Making a new class with regular control structures is favorable to deep nesting.

Allocators refers to `allocator`, this is used to wrap the resource for standard use.
`allocator<T,Resource>` and it's specialization `allocator<T,Resource*>` look very similar, but they are for different purposes.
The former should be used globally similar to how `std::allocator` is used.
The latter should be used locally, say inside of a function.
`allocator<T,Resource*>` works like `std::pmr::memory_resource` and could probably just be that.

```cpp
//               control  free block            marker    upstream        resource (also upstream)
//               |        |                     |         |               |
using resource = fallback<free_block<320, 4, 1, list<10>, local<320, 4>>, heap>; // stack allocate 10 32 byte blocks, fallback to the heap when those blocks run out.
//            allocator    resource
//            |            |
template<typename T> //    |
using alloc = allocator<T, resource>; // stateless allocator, resource is a static singleton.
```

These classes are all static.
Thus, their size can get very big (large `sizeof()`).
They are similar to arrays.
This is done so that they do not need to store any state in any memory that they have aquired, so they don't have to modify that memory in any way to maintain state. 

Allocations from any resources must be allocatable from that resource, it is an error otherwise.
That is, if a resource has been created and no allocations have been requested, the maximum size of all allocations is the maximum size that resource can fulfil.
This is to stop free blocks from allocating new memory if they cannot fulfil the request anyway.
Proper usage requires segregating those requests.
This means that the only way a resource allocation returns a `nullptr` is if it has run out of memory.
i.e. No ordering a big mac from KFC.

## Usage
Memory resources should be made and wrapped up inside one of the `allocator` classes.
Stateless allocators should be long lived and used globally, whereas stateful allocators should really be contained to local scope and should only use simple memory resources. 
You should only really consider those two types of allocators, this should ease use.

### Making memory resources
An upstream resource is picked and that is used with a `free_block` and a `marker` or a `monotonic`.

```cpp
// 1024 byte sized, 8 byte aligned request to the upstream
// 100 allocations from upstream maximum
// upstream is the heap
using resource = monotonic<1024, 8, 100, heap>; 
```

```cpp
// 10 allocations from upstream maximum
// 10 blocks per allocation, uses the pool marker (only able to allocate a single block)
// upstream is the heap (calls new and delete)
// segregator should be used, unless it is guaranteed no request over 32 bytes is ever made.
// all requests above 32 bytes returns nullptr as the nullocator always returns nullptr
using resource = segregator<32, free_block<320, 4, 10, pool<10>, heap>, nullocator>; 
```

```cpp
// the free block will allocate from the local buffer once only. Once these have all been allocated, allocate from the heap
// segregator is required here as the small allocator can only allocate upto 320 bytes.
using resource = segregator<320, free_block<320, 4, 10, list<10>, fallback<local<320,4>, heap>,heap>; 
```

```cpp
// requires two phase initialization
using resource = free_block<320, 4, 1, list<10>, buffer>;
alignas(4) char buf[320];
resource r;
r.get_upstream() = {buf, 320, 4};
```

### Making a new resource

```cpp
class resource
{
  using 32_pool = free_block<32*10,8,10,pool<10>,heap>;
  using 64_pool = free_block<64*10,8,10,pool<10>,heap>;
  using 96_pool = free_block<96*10,8,10,pool<10>,heap>;
  using 128_pool = free_block<128*5,8,5,pool<5>,heap>;
  std::variant<std::monostate, 32_pool,64_pool,96_pool,128_pool> pools[4];
  heap other;
public:
  using pointer = void *;
  using size_type = std::size_t;
  resource() noexcept
  {
    pools[0].emplace<1>();
    pools[1].emplace<2>();
    pools[2].emplace<3>();
    pools[3].emplace<4>();
  }
  pointer allocate(size_type size, size_type alignment) noexcept
  {
    auto i = size / 32;
    if(i < 4)
    {
      return std::visit([size, alignment](auto && pool) { return pool.allocate(size,alignment);}, pools[i]);
    }
    else
    {
      return other.allocate(size,alignment);
    }
  }
  void deallocate(pointer ptr, size_type bytes, size_type alignment) noexcept
  {
    auto i = bytes / 32;
    if(i < 4)
    {
      m[i].deallocate(ptr, bytes,alignment);
    }
    else
    {
      other.deallocate(ptr, bytes,alignment);
    }
  }
};
```
### Making allocators

#### Stateless Allocator

```cpp
using alloc = allocator<int, resource>; // empty class
std::vector<int, alloc> v;
```

#### Stateful Allocator

```cpp
using alloc = allocator<int, resource *>; // class contains a pointer to resource
                                          // a pointer to resource must be passed into the constructor
resource r;
std::vector<int, alloc> v(&r);
```