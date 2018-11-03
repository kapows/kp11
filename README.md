# kp11

Easy to use, policy based memory resource builder.

## Ideas

[CppCon 2015: Andrei Alexandrescu “std::allocator...”](https://www.youtube.com/watch?v=LIb3L4vKZ7U)

[CppCon 2017: Pablo Halpern “Allocators: The Good Parts”](https://www.youtube.com/watch?v=v3dz-AKOVL8)

[CppCon 2017: Bob Steagall “How to Write a Custom Allocator”](https://www.youtube.com/watch?v=kSWfushlvB8)

[C++Now 2018: Arthur O'Dwyer “An Allocator is a Handle to a Heap”](https://www.youtube.com/watch?v=0MdSJsCTRkY)

## Design

The main ideas are:

* **Split a large fixed sized memory block into smaller fixed sized memory blocks.**
Allows a single class to handle all the memory whilst deferring the block allocation strategy to other classes, which only need to deal in indexes.
* **Fixed sized alignment.**
Allows us to sidestep runtime memory alignment completely whilst also providing properly aligned memory.
* **All the resources are self contained (they don't touch the memory.)**
Allows us to treat all memory in exactly the same way. That is, a pointer to the initial memory is 
kept and we just do pointer arithmetic to allocate without embedding data in the memory itself.
* **All the resources are statically sized.**
Allows the user to know exactly how much memory will be allocated.
A limitation here is that, along with the above, there is a strict allocation limit.
The size of the resource itself could get also too big.
Resolving this would require heap allocation of the resource itself, but the user would know about it.
* **Easy to use.**
The policy based, composable nature, make creating and adapting different resources easy.
If more complex control structures than the ones provided are needed, then the user should create a new resource class.
If more complex construction is required, then it has to be done in the two-phase initialization style.

## Use

Steps:
1. Choose `free_block` or `monotonic`. 
2. Pick a `Marker` (if using `free_block`).
3. Pick an "Upstream" `Resource`.
4. Possibly guard against the allocation limitation with `fallback`.
5. Possibly guard against the `Marker`'s or `monotonic` limitations with a `segregator`.

```cpp
#include <kp11/allocator.h> // Adaptor
#include <kp11/fallback.h> // A control structure to deal free_block allocation limitations
#include <kp11/free_block.h> // Our main block splitting class
#include <kp11/heap.h> // An Upstream resource
#include <kp11/pool.h> // One of our `Marker`s
#include <kp11/segregator.h> // A control structure to deal with the limitations of the pool

#include <string>

using namespace kp11;
// This resource uses a pool allocation strategy.
// It allocates 10 256 byte blocks from heap in a single allocation (2560 bytes) (up to 5 times).
using pool_256 = free_block<256 * 10, alignof(std::max_align_t), 5, pool<10>, heap>;

// This resource allocates from the heap when all the small blocks run out.
// In this case it is 10 * 5 small block allocations without deallocations.
using safe_pool_256 = fallback<pool_256, heap>;

// This resource segregates allocations <= 256 to safe_pool_256 all others go to heap.
// This resource can now support any allocations
using resource = segregator<256, safe_pool_256, heap>;

// creates an allocator with a standard interface
template<typename T>
using alloc = allocator<T, resource>;

using string = std::basic_string<char, std::char_traits<char>, alloc<char>>;

int main()
{
  // Allocates from our pool
  string s = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
             "incididunt ut labore et dolore magna aliqua.";
  return 0;
}
```

Though control structures are provided, you don't have to use them and can just use regular C++.

```cpp
#include <kp11/fallback.h> // A control structure to deal free_block allocation limitations
#include <kp11/free_block.h> // Our main block splitting class
#include <kp11/heap.h> // An Upstream resource
#include <kp11/pool.h> // One of our `Marker`s

#include <cstddef>
#include <variant>

using namespace kp11;

class resource
{
  using pool_32 = free_block<32 * 10, alignof(std::max_align_t), 10, pool<10>, heap>;
  using pool_64 = free_block<64 * 10, alignof(std::max_align_t), 10, pool<10>, heap>;
  using pool_96 = free_block<96 * 10, alignof(std::max_align_t), 10, pool<10>, heap>;
  using pool_128 = free_block<128 * 10, alignof(std::max_align_t), 5, pool<10>, heap>;
  std::variant<pool_32, pool_64, pool_96, pool_128> pools[4];
  heap my_heap;

public:
  using pointer = typename heap::pointer;
  using size_type = typename heap::size_type;

  resource()
  {
    pools[0].emplace<pool_32>();
    pools[1].emplace<pool_64>();
    pools[2].emplace<pool_96>();
    pools[3].emplace<pool_128>();
  }
  pointer allocate(size_type size, size_type alignment) noexcept
  {
    if (auto i = index_for(size); i < 4)
    {
      auto p = std::visit(
        [size, alignment](auto && p) { return p.allocate(size, alignment); }, pools[size / 32]);
      if (p == nullptr)
      {
        p = my_heap.allocate(size, alignment);
      }
      return p;
    }
    return my_heap.allocate(size, alignment);
  }
  void deallocate(pointer ptr, size_type size, size_type alignment) noexcept
  {
    if (auto i = index_for(size); i < 4)
    {
      if (!std::visit(
            [ptr, size, alignment](auto && p) { return p.deallocate(ptr, size, alignment); },
            pools[size / 32]))
      {
        my_heap.deallocate(ptr, size, alignment);
      };
    }
    else
    {
      my_heap.deallocate(ptr, size, alignment);
    }
  }

private:
  static constexpr std::size_t index_for(size_type size) noexcept
  {
    return size == 0 ? 0 : size / 32 - (size % 32 == 0);
  }
};
```

## Install

### Shell

```Shell
git clone https://github.com/kapows/kp11 && cd kp11
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_CXX_FLAGS=-fsized-deallocation -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH
cmake --build . --target install --config Release
```

### Cmake

```Cmake
find_package(kp11 CONFIG REQUIRED)
target_link_libraries(main PRIVATE kp11::kp11)
```

## Develop

### Shell

```Shell
vcpkg install catch2
git clone https://github.com/kapows/kp11 && cd kp11
mkdir build && cd build
cmake .. -G Ninja -DBUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE=$VCPKG_PATH -DCMAKE_CXX_FLAGS=-fsized-deallocation
cmake --build . --config Debug
ctest
```