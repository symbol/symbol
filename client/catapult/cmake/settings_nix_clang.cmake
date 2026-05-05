# We don't support Clang < 14'
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
	message(FATAL_ERROR "Clang version must be at least 14.0! Found version ${CMAKE_CXX_COMPILER_VERSION}")
endif()


target_compile_options(build.defaults INTERFACE 
	$<$<BOOL:${ARCHITECTURE_NAME}>:-march=${ARCHITECTURE_NAME}>		
	-Weverything									# enable all warnings
	-Werror											# treat warnings as errors
	-Wno-c++98-compat								# catapult is not compatible with C++98
	-Wno-c++98-compat-pedantic						# catapult is not compatible with C++98
	-Wno-disabled-macro-expansion					# expansion of recursive macro is required
	-Wno-padded										# allow compiler to automatically pad data types for alignment
	-Wno-switch-enum								# do not require enum switch statements to list every value
	-Wno-weak-vtables								# vtables are emitted in all translation units for virtual classes with no out-of-line virtual method definitions
	-Wno-shadow-uncaptured-local					# allow shadowing of local variables in lambdas https://github.com/llvm/llvm-project/issues/81307
	$<$<VERSION_GREATER_EQUAL:${CMAKE_CXX_COMPILER_VERSION},16.0>:-Wno-unsafe-buffer-usage>			# allow unsafe buffer usage https://reviews.llvm.org/D137379

	$<$<BOOL:${ENABLE_CODE_COVERAGE}>:-fprofile-instr-generate>
	$<$<BOOL:${ENABLE_CODE_COVERAGE}>:-fcoverage-mapping>

	$<$<BOOL:${USE_SANITIZER}>:-fno-omit-frame-pointer>
	$<$<BOOL:${USE_SANITIZER}>:-fsanitize=${USE_SANITIZER}>
	$<$<BOOL:${USE_SANITIZER}>:-fsanitize-ignorelist=${PROJECT_SOURCE_DIR}/sanitizer_ignorelist.txt>
	$<$<STREQUAL:${USE_SANITIZER},undefined>:-fsanitize=implicit-conversion,nullability>
	$<$<AND:$<STREQUAL:${USE_SANITIZER},undefined>,$<BOOL:${ENABLE_FUZZ_BUILD}>>:-fsanitize=address>
	$<$<AND:$<STREQUAL:${USE_SANITIZER},undefined>,$<BOOL:${ENABLE_FUZZ_BUILD}>>:-fno-sanitize-recover=all>
	$<$<AND:$<STREQUAL:${USE_SANITIZER},undefined>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},arm64>>:-fno-sanitize=vptr>

    $<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-D_FORTIFY_SOURCE=3>
	$<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-fstack-protector-all>
	$<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-fsanitize=safe-stack>

	$<$<CONFIG:RelWithDebInfo>:-g1>
)

target_link_options(build.defaults INTERFACE
	$<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-fsanitize=safe-stack>
	$<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-Wl,-z,noexecstack>   # NX bit - prevent code execution from stack
	$<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-Wl,-z,relro>         # Read-only relocation - make reloc section read-only
	$<$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-Wl,-z,now>           # Resolve all symbols immediately (no lazy binding)
)

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
    $<$<BOOL:${ENABLE_TESTS}>:-Wno-global-constructors>
	$<$<BOOL:${ENABLE_TESTS}>:-Wno-zero-as-null-pointer-constant>
	$<$<BOOL:${ENABLE_TESTS}>:-Wno-missing-noreturn>
)
