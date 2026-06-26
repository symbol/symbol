# We don't support GCC < 8'
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0")
	message(FATAL_ERROR "GCC version must be at least 8.0! Found version ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# We strongly encourage GCC >= 11
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
	message(WARNING "Your GCC version ${CMAKE_CXX_COMPILER_VERSION} is quite old. Please upgrade to 11 or better")
endif()

target_compile_definitions(build.defaults INTERFACE 
	__STDC_WANT_LIB_EXT1__=1
)

target_compile_options(build.defaults INTERFACE 
	$<$<BOOL:${ARCHITECTURE_NAME}>:-march=${ARCHITECTURE_NAME}>
	$<$<COMPILE_LANGUAGE:CXX>:-Wall>											# enable all warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Wextra>											# enable extra warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>										# enable pedantic warnings
	$<$<COMPILE_LANGUAGE:CXX>:-Wshadow>											# warn when a local variable shadows another variable
	$<$<COMPILE_LANGUAGE:CXX>:-Wconversion>										# warn on type conversions that may change a value
	$<$<COMPILE_LANGUAGE:CXX>:-Wformat-security>								# warn about format string vulnerabilities
	$<$<COMPILE_LANGUAGE:CXX>:-Werror>											# treat warnings as errors
	$<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>								# perform most pedantic strict aliasing checks
	$<$<COMPILE_LANGUAGE:CXX>:-Wno-maybe-uninitialized>							# allow uninitialized variables

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-CODE-COVERAGE}>>:--coverage>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-CODE-COVERAGE}>>:-fprofile-arcs>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-CODE-COVERAGE}>>:-ftest-coverage>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-CODE-COVERAGE}>>:-fprofile-update=atomic>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-SANITIZER}>>:-fno-omit-frame-pointer>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-SANITIZER}>>:-fsanitize=${W-SANITIZER}>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${W-SANITIZER}>>:-fsanitize-ignorelist=${PROJECT_SOURCE_DIR}/sanitizer_ignorelist.txt>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${W-SANITIZER},undefined>>:-fsanitize=implicit-conversion,nullability>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${W-SANITIZER},undefined>,$<BOOL:${W-FUZZ-BUILD}>>:-fsanitize=address>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${W-SANITIZER},undefined>,$<BOOL:${W-FUZZ-BUILD}>>:-fno-sanitize-recover=all>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:${W-SANITIZER},undefined>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>,$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},arm64>>:-fno-sanitize=vptr>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>>:-U_FORTIFY_SOURCE>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>>:-D_FORTIFY_SOURCE=3>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>>:-fstack-protector-all>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>>:-fstack-clash-protection>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-g1>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-fno-omit-frame-pointer>

	$<$<COMPILE_LANGUAGE:C>:-Wno-deprecated-declarations>
)

target_link_libraries(build.defaults INTERFACE
	$<$<AND:$<VERSION_GREATER_EQUAL:${CMAKE_CXX_COMPILER_VERSION},8.0>,$<VERSION_LESS:${CMAKE_CXX_COMPILER_VERSION},9.0>>:stdc++fs>
)

target_link_options(build.defaults INTERFACE
	$<$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-Wl,-z,noexecstack>
	$<$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-Wl,-z,relro>
	$<$<OR:$<BOOL:${W-HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>:-Wl,-z,now>
)

target_compile_options(build.tests INTERFACE
	$<$<AND:$<BOOL:${W-TESTS}>,$<COMPILE_LANGUAGE:CXX>>:-Wno-dangling-else>
	$<$<AND:$<BOOL:${W-TESTS}>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:${CMAKE_CXX_COMPILER_VERSION},13.0>>:-Wno-dangling-reference>
	$<$<AND:$<BOOL:${W-TESTS}>,$<COMPILE_LANGUAGE:CXX>,$<VERSION_GREATER_EQUAL:${CMAKE_CXX_COMPILER_VERSION},14.0>,$<VERSION_LESS:${CMAKE_CXX_COMPILER_VERSION},16.0>>:-Wno-free-nonheap-object>
)

target_link_libraries(build.tests INTERFACE
	$<$<BOOL:${W-TESTS}>:${GTest_IMPORTED_TARGETS}>
)
