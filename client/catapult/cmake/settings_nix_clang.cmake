# We don't support Clang < 14'
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
	message(FATAL_ERROR "Clang version must be at least 14.0! Found version ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# Set parallel build level for all generators (Make, Ninja, etc.)
set(CMAKE_BUILD_PARALLEL_LEVEL ${PARALLEL_BUILDS})

target_compile_definitions(build.defaults INTERFACE 
	$<$<BOOL:${ARCHITECTURE_NAME}>:-march=${ARCHITECTURE_NAME}>
)

target_compile_options(build.defaults INTERFACE 
	$<$<COMPILE_LANGUAGE:CXX>:-Weverything>										# enable all warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Werror>											# treat warnings as errors
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-c++98-compat>								# catapult is not compatible with C++98
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-c++98-compat-pedantic>						# catapult is not compatible with C++98
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-disabled-macro-expansion>					# expansion of recursive macro is required
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-padded>										# allow compiler to automatically pad data types for alignment
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-switch-enum>									# do not require enum switch statements to list every value
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-weak-vtables>								# vtables are emitted in all translation units for virtual classes with no out-of-line virtual method definitions
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-shadow-uncaptured-local>						# allow shadowing of local variables in lambdas https://github.com/llvm/llvm-project/issues/81307
	$<$<BOOL:${ARCHITECTURE_NAME}>:-march=${ARCHITECTURE_NAME}>		
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,16.0>>:-Wno-unsafe-buffer-usage>      # allow unsafe buffer usage https://reviews.llvm.org/D137379

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_CODE_COVERAGE}>>:-fprofile-instr-generate>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_CODE_COVERAGE}>>:-fcoverage-mapping>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${USE_SANITIZER}>>:-fno-omit-frame-pointer>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${USE_SANITIZER}>>:-fsanitize=${USE_SANITIZER}>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${USE_SANITIZER}>>:-fsanitize-ignorelist=${PROJECT_SOURCE_DIR}/sanitizer_ignorelist.txt>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>>:-fsanitize=implicit-conversion,nullability>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>,$<BOOL:${ENABLE_FUZZ_BUILD}>>:-fsanitize=address>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>,$<BOOL:${ENABLE_FUZZ_BUILD}>>:-fno-sanitize-recover=all>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},arm64>>:-fno-sanitize=vptr>

	$<$<AND:$<BOOL:${ENABLE_HARDENING}>,$<COMPILE_LANGUAGE:CXX>>:-D_FORTIFY_SOURCE=3>
	$<$<AND:$<BOOL:${ENABLE_HARDENING}>,$<COMPILE_LANGUAGE:CXX>>:-fstack-protector-all>
	$<$<AND:$<BOOL:${ENABLE_HARDENING}>,$<COMPILE_LANGUAGE:CXX>>:-fsanitize=safe-stack>

	$<$<CONFIG:RelWithDebInfo>:-g1>
)

if(ENABLE_HARDENING)
	target_link_options(build.defaults INTERFACE
		-fsanitize=safe-stack
		-Wl,-z,noexecstack   # NX bit - prevent code execution from stack
		-Wl,-z,relro         # Read-only relocation - make reloc section read-only
		-Wl,-z,now           # Resolve all symbols immediately (no lazy binding)
	)
endif()


# fix -Wpoison-system-directories: error: include location '/usr/local/include' is "unsafe for cross-compilation"
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" AND NOT DEFINED CMAKE_OSX_SYSROOT)
	find_program(XCODE_SELECT xcode-select)
	if (XCODE_SELECT)
		execute_process(COMMAND xcode-select --print-path OUTPUT_VARIABLE XCODE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE XCODE_SELECT_RESULT)
		if(XCODE_SELECT_RESULT EQUAL 0 AND XCODE_PATH)
			set(CMAKE_OSX_SYSROOT "${XCODE_PATH}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
		else()
			message(FATAL_ERROR "xcode-select failed with exit code ${XCODE_SELECT_RESULT}, cannot automatically set CMAKE_OSX_SYSROOT.")
		endif()
	else()
		message(FATAL_ERROR "xcode-select not found, cannot automatically set CMAKE_OSX_SYSROOT.")
	endif()
endif()

target_compile_options(build.tests INTERFACE
	# - Wno-global-constructors: required for GTEST test definition macros
	# - Wno-zero-as-null-pointer-constant: workaround for GTEST NULL/nullptr mismatch https://github.com/google/googletest/issues/1323
    $<$<AND:$<BOOL:${ENABLE_TESTS}>,$<COMPILE_LANGUAGE:CXX>>:-Wno-global-constructors>
    $<$<AND:$<BOOL:${ENABLE_TESTS}>,$<COMPILE_LANGUAGE:CXX>>:-Wno-zero-as-null-pointer-constant>
)
