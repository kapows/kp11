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

## Usage
Memory resource should be made and wrapped up inside one of the `allocator` classes.
Stateless allocators should be long lived and used globally, whereas stateful allocators should really be contained to local scope and should only use simple memory resources.

### Making memory resources
An upstream resource is picked and that is used with a `free_block` and a `marker` or a `monotonic`.

#### Examples

##### Simple
```cpp
using resource = free_block<10, pool<250>, heap>; // 10 allocations from upstream maximum
                                                  // has 250 blocks per allocation, uses the pool marker (only able to allocate a single block)
                                                  // upstream is the heap
resource r(250*40, 4); // allocates 250*40 bytes from upstream, that is each pool block is 40 bytes.
```
```cpp
using resource = monotonic<100, heap>; // 100 allocations from upstream maximum
                                       // upstream is the heap
resource r(250*40, 4); // allocates 250*40 bytes from upstream
```
```cpp
using resource = fallback<free_block<1, pool<100>, local<32*100,4>>, heap>; // the free block will allocate from the local buffer once only. Once these have all been allocated, allocate from the heap
```
```cpp
using resource = segregator<32, free_block<10, pool<100>, heap>, heap>; // anything bigger than 32 bytes will be allocated from the heap
```
Try not to use too many of these control structures together as their constructors get really complicated.

##### Complicated
```cpp
using resource = segregator<32, free_block<10,pool<100>,heap>, segragator<64, free_block<10,pool<100>, heap>, heap>>>; // don't do this
// Instead just make your own resource
class resource
{
public:
  using pointer = void *;
  using size_type = std::size_t;
  resource() {
    p.emplace_back(32*100, 4); 
    p.emplace_back(64*100, 4);
  }
  pointer allocate(size_type bytes, size_type alignment)
  {
    auto i = bytes / 32;
    if(i < m.size())
    {
      return m[i].allocate(bytes,alignment);
    }
    else
    {
      return other.allocate(bytes,alignment);
    }
  }
  void deallocate(pointer ptr, size_type bytes, size_type alignment)
  {
    auto i = bytes / 32;
    if(i < m.size())
    {
      m[i].deallocate(ptr, bytes,alignment);
    }
    else
    {
      other.deallocate(ptr, bytes,alignment);
    }
  }
private:
  std::vector<free_block<10, pool<100>, heap>> m;
  heap other;
};
```
### Making allocators

#### Examples

##### Stateless Allocator
```cpp
using alloc = allocator<int, resource>; // empty class
std::vector<int, alloc> v;
```
##### Stateful Allocator
```cpp
using alloc = allocator<int, resource *>; // class contains a pointer to resource
                                          // a pointer to resource must be passed into the constructor
resource r;
std::vector<int, alloc> v(&r);
```