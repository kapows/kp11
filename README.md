# kp11

Easy to use, policy based memory resource builder.

## Ideas

[CppCon 2015: Andrei Alexandrescu “std::allocator...”](https://www.youtube.com/watch?v=LIb3L4vKZ7U)

[CppCon 2017: Pablo Halpern “Allocators: The Good Parts”](https://www.youtube.com/watch?v=v3dz-AKOVL8)

[CppCon 2017: Bob Steagall “How to Write a Custom Allocator”](https://www.youtube.com/watch?v=kSWfushlvB8)

[C++Now 2018: Arthur O'Dwyer “An Allocator is a Handle to a Heap”](https://www.youtube.com/watch?v=0MdSJsCTRkY)

## Design

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