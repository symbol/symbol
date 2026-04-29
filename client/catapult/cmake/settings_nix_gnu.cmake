# We don't support GCC < 8'
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
	message(FATAL_ERROR "GCC version must be at least 8.0! Found version ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# We strongly encourage GCC >= 11
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
	message(WARNING "Your GCC version ${CMAKE_CXX_COMPILER_VERSION} is quite old. Please upgrade to 11 or better")
endif()

target_compile_definitions(build.defaults INTERFACE 
	$<$<BOOL:${ARCHITECTURE_NAME}>:-march=${ARCHITECTURE_NAME}>
	_STDC_WANT_LIB_EXT1_=1
	__STDC_WANT_LIB_EXT1__=1
)

target_compile_options(build.defaults INTERFACE 
	$<$<COMPILE_LANGUAGE:CXX>:-Wall>											# enable all warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Wextra>											# enable extra warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>										# enable pedantic warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Wshadow>											# warn when a local variable shadows another variable
	$<$<COMPILE_LANGUAGE:CXX>:-Wconversion>										# warn on type conversions that may change a value
	$<$<COMPILE_LANGUAGE:CXX>:-Wformat-security>								# warn about format string vulnerabilities
	$<$<COMPILE_LANGUAGE:CXX>:-Werror>											# treat warnings as errors
	$<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>								# perform most paranoid strict aliasing checks
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-maybe-uninitialized>							# allow uninitialized variables

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_CODE_COVERAGE}>>:--coverage>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_CODE_COVERAGE}>>:-fprofile-arcs>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_CODE_COVERAGE}>>:-ftest-coverage>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_CODE_COVERAGE}>>:-fprofile-update=atomic>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${USE_SANITIZER}>>:-fno-omit-frame-pointer>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${USE_SANITIZER}>>:-fsanitize=${USE_SANITIZER}>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${USE_SANITIZER}>>:-fsanitize-ignorelist=${PROJECT_SOURCE_DIR}/sanitizer_ignorelist.txt>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>>:-fsanitize=implicit-conversion,nullability>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>,$<BOOL:${ENABLE_FUZZ_BUILD}>>:-fsanitize=address>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>,$<BOOL:${ENABLE_FUZZ_BUILD}>>:-fno-sanitize-recover=all>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${USE_SANITIZER},undefined>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},arm64>>:-fno-sanitize=vptr>

	$<$<AND:$<BOOL:${ENABLE_HARDENING}>,$<COMPILE_LANGUAGE:CXX>>:-D_FORTIFY_SOURCE=3> # Don't pass this using add_option as GCC complains _FORTIFY_SOURCE has been redefined
	$<$<AND:$<BOOL:${ENABLE_HARDENING}>,$<COMPILE_LANGUAGE:CXX>>:-fstack-protector-all>
	$<$<AND:$<BOOL:${ENABLE_HARDENING}>,$<COMPILE_LANGUAGE:CXX>>:-fstack-clash-protection>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-g1>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-fno-omit-frame-pointer>

	$<$<COMPILE_LANGUAGE:C>:-Wno-deprecated-declarations>
)

target_link_options(build.defaults INTERFACE
	$<$<CXX_COMPILER_VERSION,8>:stdc++fs>
	$<$<BOOL:${ENABLE_HARDENING}>:-Wl,-z,noexecstack>	# NX bit - prevent code execution from stack
	$<$<BOOL:${ENABLE_HARDENING}>:-Wl,-z,relro>         # Read-only relocation - make reloc section read-only
	$<$<BOOL:${ENABLE_HARDENING}>:-Wl,-z,now>           # Resolve all symbols immediately (no lazy binding)
)

target_compile_options(build.tests INTERFACE
	# - Wno-dangling-else: workaround for GTEST ambiguous else blocker not working https://github.com/google/googletest/issues/1119
	# disable dangling reference for tests - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108165#c9
    $<$<AND:$<BOOL:${ENABLE_TESTS}>,$<COMPILE_LANGUAGE:CXX>>:-Wno-dangling-else>
    $<$<AND:$<BOOL:${ENABLE_TESTS}>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,13.0>>:-Wno-dangling-reference>
)
