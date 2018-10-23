cmake_minimum_required(VERSION 3.8)

project ("kp11" 
    LANGUAGES CXX
    VERSION 0.0.0)

set(INSTALL_INCLUDE_DIR include)
set(INSTALL_ARCHIVE_DIR lib/kp11)
set(INSTALL_CONFIG_DIR lib/cmake/kp11)
set(EXPORT_TARGET kp11-targets)
set(CONFIG_NAME kp11-config)

add_library(kp11 STATIC
    include/kp11/heap.h src/heap.cpp
    include/kp11/stack.h
    include/kp11/free_block.h
    include/kp11/pool.h
    include/kp11/list.h
    include/kp11/bitset.h
    include/kp11/local.h
    include/kp11/monotonic.h
    include/kp11/fallback.h
    include/kp11/allocator.h
    include/kp11/detail/static_vector.h
    include/kp11/segregator.h
    include/kp11/nullocator.h
    )
add_library(kp11::kp11 ALIAS kp11)
target_compile_features(kp11 PUBLIC cxx_std_17)
target_include_directories(kp11 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/kp11>
    $<INSTALL_INTERFACE:include>
    )

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory("include/kp11")
endif()

install(DIRECTORY include/ 
	DESTINATION ${INSTALL_INCLUDE_DIR}
	FILES_MATCHING PATTERN "*.h"
	)
install(TARGETS kp11 
	EXPORT ${EXPORT_TARGET}   
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR}
    )

# INSTALL TARGETS
install(EXPORT ${EXPORT_TARGET}
	NAMESPACE kp11:: 
	FILE ${EXPORT_TARGET}.cmake 
	DESTINATION ${INSTALL_CONFIG_DIR}
	)
# INSTALL CONFIG 
include(CMakePackageConfigHelpers)
# CONFIG
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/${CONFIG_NAME}.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${CONFIG_NAME}.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIG_DIR}
	)
# VERSION
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/${CONFIG_NAME}-version.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
	)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${CONFIG_NAME}.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/${CONFIG_NAME}-version.cmake"
  DESTINATION
    ${INSTALL_CONFIG_DIR}
	)