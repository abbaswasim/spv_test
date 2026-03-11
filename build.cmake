if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo) # good compromise
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

if(BUILD_SANITIZED AND (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
	set(SPV_TEST_SANITIZE_COMPILE_OPTS
		-fsanitize=undefined
		-fno-sanitize-recover=undefined
		-fno-omit-frame-pointer
	)

	set(SPV_TEST_SANITIZE_LINK_OPTS
		-fsanitize=undefined
	)

	list(APPEND SPV_TEST_SANITIZE_COMPILE_OPTS -fsanitize=address -fstack-protector)
	list(APPEND SPV_TEST_SANITIZE_LINK_OPTS -fsanitize=address -fstack-protector)

	# Build sanitized code consistently across our deps too, otherwise libc++'s ASan container
	# annotations can get out of sync when passing STL containers across TUs.
	set(SPV_TEST_SANITIZE_TARGETS
		spv_test
		glslang
		glslang-default-resource-limits
		spirv-cross-core
		spirv-cross-glsl
		spirv-cross-msl
		spirv-cross-util
		spirv-cross-cpp
		spirv-cross-reflect
		spirv-cross-c
	)

	foreach(target_name IN LISTS SPV_TEST_SANITIZE_TARGETS)
		if(TARGET ${target_name})
			target_compile_options(${target_name} PRIVATE ${SPV_TEST_SANITIZE_COMPILE_OPTS})
		endif()
	endforeach()

	target_link_options(spv_test PRIVATE ${SPV_TEST_SANITIZE_LINK_OPTS})

endif()

target_compile_options(spv_test PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wconversion
  -Wsign-conversion
  -Wunreachable-code
  -Wuninitialized
  -pedantic-errors
  -Wold-style-cast
  -Wshadow
  -Weffc++
  -Wfloat-equal)

  target_compile_options(spv_test PRIVATE
	-Wmost
	-Wno-c++98-compat-pedantic
	-Wno-c++98-compat
	-Wno-switch-default
	-Wno-missing-include-dirs
	-Wno-poison-system-directories
	-Wno-gnu-zero-variadic-macro-arguments)

  target_compile_options(spv_test PRIVATE
	-Weverything
	-Wno-exit-time-destructors 
	-Wno-global-constructors   
	-Wno-c++2a-compat
	-Wno-missing-prototypes
	-Wno-padded                
	-Wno-double-promotion      
	-Wno-documentation         
  )
