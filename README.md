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
A limitation here is that, along with the above, the size of the resource itself could get too big.
Resolving this would require heap allocation of the resource itself, but the user would know about it.
* **Unchecked allocations.**
Requires the user to segregate allocations that resources cannot handle, it is undefined behaviour if these allocations go through.
This is only checked with asserts.
This allows the user to know that allocation failure is the result of some limitation being met (out of memory, out of small blocks, out of big blocks), rather than as a result of requesting memory that the resource could never have fulfilled.
i.e. You can't order a Big Mac from KFC.
* **Easy to use.**
The policy based, composable nature, make creating and adapting different resources easy.
If more complex control structures than the ones provided are needed, then the user should create a new resource class.
If more complex construction is required, then it has to be done in the two-phase initialization style.

## Use

```Cmake
find_package(kp11 CONFIG REQUIRED)
target_link_libraries(main PRIVATE kp11::kp11)
```

## Install

```Shell
git clone https://github.com/kapows/kp11 && cd kp11
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_CXX_FLAGS=-fsized-deallocation -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH
cmake --build . --target install --config Release
```

## Develop

```Shell
vcpkg install catch2
git clone https://github.com/kapows/kp11 && cd kp11
mkdir build && cd build
cmake .. -G Ninja -DBUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE=$VCPKG_PATH -DCMAKE_CXX_FLAGS=-fsized-deallocation
cmake --build . --config Debug
ctest
```