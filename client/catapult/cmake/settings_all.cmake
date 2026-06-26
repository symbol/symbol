### enable testing
enable_testing()

# Support for 64-bit Little Endian architectures only
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8 OR NOT CMAKE_CXX_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
	math(EXPR _bitness "${CMAKE_SIZEOF_VOID_P} * 8")
	message(FATAL_ERROR 
		"This project requires a 64-bit Little Endian operating system and compiler.\n"
		"You're currently using a ${_bitness}-bit ${CMAKE_CXX_BYTE_ORDER} Endian architecture."
	)
endif()

### enable ccache by default if available
if(NOT W-NO-CCACHE)
	find_program(CCACHE_BIN ccache)
	if(CCACHE_BIN)
		set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_BIN}")
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_BIN}")
		set(CMAKE_C_LINKER_LAUNCHER "${CCACHE_BIN}")
		set(CMAKE_CXX_LINKER_LAUNCHER "${CCACHE_BIN}")
	endif()
endif()

if(CATAPULT_BUILD_RELEASE)
	set(W-HARDENING ON)
endif()

### set up conan
set(CONAN_SYSTEM_INCLUDES $<BOOL:${USE_CONAN}>)

# Dynamically determine number of cores for parallel builds
include(ProcessorCount)
ProcessorCount(NUM_CORES)
# Use 80% of available cores for parallel compilation
math(EXPR PARALLEL_BUILDS "${NUM_CORES} * 80 / 100")
if(PARALLEL_BUILDS LESS 1)
	set(PARALLEL_BUILDS 1)
endif()
set(CMAKE_BUILD_PARALLEL_LEVEL ${PARALLEL_BUILDS})

set(CORE_CATAPULT_LIBS catapult.io catapult.ionet catapult.model catapult.thread catapult.utils)

# set CATAPULT_VERSION_DESCRIPTION to a reasonable value
if(NOT CATAPULT_BUILD_DEVELOPMENT)
	# extract version information from git
	execute_process(
		COMMAND git rev-parse --abbrev-ref HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(
		COMMAND git log -1 --format=%h
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	if(CATAPULT_BUILD_RELEASE_PUBLIC)
		set(CATAPULT_VERSION_DESCRIPTION "(public)")
	else()
		set(CATAPULT_VERSION_DESCRIPTION "${GIT_COMMIT_HASH} [${GIT_BRANCH}]")
	endif()
endif()

# on Linux always set explicit rpaths; on macOS only when using conan
# (conan copies dependencies to `@executable_path/../deps`)
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux" OR (USE_CONAN AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
	set(ENABLE_RPATHS ON)
	set(USE_EXPLICIT_RPATHS ON)
endif()

if(ENABLE_RPATHS)
	if(USE_EXPLICIT_RPATHS)
		if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
			# build tree: $ORIGIN so all shared libs in bin/ are found without LD_LIBRARY_PATH
			set(CMAKE_BUILD_RPATH "$ORIGIN")
			set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
			# install tree: locate plugins and dependencies relative to the executable
			set(CMAKE_INSTALL_RPATH "$ORIGIN/../deps:$ORIGIN/../lib")
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

			# use rpath for executables
			# (executable rpath will be used for loading indirect libs, this is needed because boost libs do not set runpath)
			# use newer runpath for shared libs
			# exclude-libs for RocksDB: prevent RocksDB static-archive symbols from being exported from each DSO,
			# avoiding ELF symbol preemption / double-free at exit while keeping all other symbols (e.g. boost_log) global
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags,--exclude-libs,librocksdbd.a:librocksdb.a")
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
		endif()
		if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			set(CMAKE_INSTALL_RPATH "@executable_path/../deps;@executable_path/../lib")
			set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
		endif()
	endif()
else()
	set(CMAKE_SKIP_BUILD_RPATH TRUE)
endif()

if(MSVC)
	# embed the version rc file
	set(VERSION_RESOURCES ${CMAKE_SOURCE_DIR}/src/catapult/version/win/win_version.rc)
else()
	# since strings are not referenced, in order for the linker to include them, they must be forcibly linked via an object file
	set(VERSION_RESOURCES $<TARGET_OBJECTS:catapult.version.nix>)
endif()

# Create interface libraries for compiler settings
add_library(build.defaults INTERFACE)
add_library(build.tests INTERFACE)

# Common compiler settings for all builds
target_compile_definitions(build.defaults INTERFACE 
	DLL_EXPORTS
	BOOST_ALL_DYN_LINK
	BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT
	BOOST_ASIO_NO_DEPRECATED

	# Boost.Asio is compiled once into a dedicated shared library (catapult.asio) so that every module shares a single
	# instance of Asio's runtime and thread-local state, which fixes TLS destructor-ordering crashes on shutdown.
	# This relies on the ELF dynamic-linking model: with default symbol visibility the loader interposes all duplicate
	# copies of Asio's header-only template statics (e.g. detail::call_stack<>::top_) onto a single definition.
	#
	# Windows (PE) has no such interposition: cross-module symbol identity requires explicit dllexport/dllimport, which
	# Asio only applies to its non-template functions, not to its template/static TLS state. The result is a split-brain
	# where imported Asio functions and per-module template TLS disagree, causing access violations on io-pool shutdown.
	# This is a property of the PE format and Windows loader (independent of the compiler: MSVC, MinGW or clang-cl all
	# target PE), so the shared-Asio design is gated to non-Windows targets; on Windows Asio is kept header-only.
	$<$<NOT:$<BOOL:${WIN32}>>:BOOST_ASIO_SEPARATE_COMPILATION>
	$<$<NOT:$<BOOL:${WIN32}>>:BOOST_ASIO_DYN_LINK>

	OPENSSL_API_COMPAT=0x10100000L
	$<$<BOOL:${W-CATAPULT-DIAGNOSTICS}>:ENABLE_CATAPULT_DIAGNOSTICS>
	$<$<BOOL:${CATAPULT_DOCKER_TESTS}>:CATAPULT_DOCKER_TESTS>
	$<$<BOOL:${CATAPULT_TEST_DB_URL}>:CATAPULT_TEST_DB_URL="${CATAPULT_TEST_DB_URL}">
)

target_include_directories(build.defaults SYSTEM BEFORE INTERFACE 
	${Boost_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/
)
