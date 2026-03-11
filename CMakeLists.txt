cmake_minimum_required (VERSION 3.12)

project(spv_test VERSION 1.1.0.0 LANGUAGES CXX C)
list(APPEND CMAKE_MESSAGE_CONTEXT SPV_TEST)

set(SPV_TEST_NAME SPV_TEST)

find_package(Threads REQUIRED)

set(SPV_TEST_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SPV_TEST_SOURCE_DIR ${SPV_TEST_ROOT_DIR})

set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${SPV_TEST_ROOT_DIR}/thirdparty/spirv-cross)

set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
add_subdirectory(${SPV_TEST_ROOT_DIR}/thirdparty/glslang)

add_executable(spv_test
	${SPV_TEST_SOURCE_DIR}/spv_test.cpp
)

# add_executable(spv_test_pthreads
# 	${SPV_TEST_SOURCE_DIR}/spv_test_pthreads.cpp
# )

set_target_properties(spv_test PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

target_link_libraries(spv_test PRIVATE
	$<LINK_ONLY:spirv-cross-msl>
	$<LINK_ONLY:glslang::glslang>
	$<LINK_ONLY:glslang::glslang-default-resource-limits>
	Threads::Threads
)

target_compile_definitions(spv_test PRIVATE
	SPV_TEST_FRAG_PATH="${SPV_TEST_SOURCE_DIR}/test.frag"
)

if(BUILD_PARALLEL)
	target_compile_definitions(spv_test PRIVATE	BUILD_PARALLEL=1)
endif(BUILD_PARALLEL)

include(${SPV_TEST_ROOT_DIR}/build.cmake)
