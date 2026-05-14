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

### enable ccache if available
find_program(CCACHE_EXE ccache)
if(CCACHE_EXE)
	# ccache on windows requires real binary instead of the shims used by scoop to be in the PATH
	# Covers both cl (MSVC) and clang-cl (Clang with MSVC frontend)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		set(_msvc_frontend_tool "clang-cl.exe")
	elseif(MSVC)
		set(_msvc_frontend_tool "cl.exe")
	endif()

	if(_msvc_frontend_tool AND ENABLE_CCACHE_ON_WINDOWS)
		file(COPY_FILE ${CCACHE_EXE} ${CMAKE_BINARY_DIR}/${_msvc_frontend_tool} ONLY_IF_DIFFERENT)
		set(CMAKE_VS_GLOBALS
			"CLToolExe=${_msvc_frontend_tool}"
			"CLToolPath=${CMAKE_BINARY_DIR}"
			"TrackFileAccess=false"
			"UseMultiToolTask=true"
			"DebugInformationFormat=OldStyle"
		)
	else()
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
	endif()
endif(CCACHE_EXE)

if(CATAPULT_BUILD_RELEASE)
	set(ENABLE_HARDENING ON)
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

# only set rpath when running conan, which copies dependencies to `@executable_path/../deps`
# when not using conan, rpath is set to link paths by default
if(USE_CONAN AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(ENABLE_RPATHS ON)
	set(USE_EXPLICIT_RPATHS ON)
endif()

if(ENABLE_RPATHS)
	if(USE_EXPLICIT_RPATHS)
		if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
			# $origin - to load plugins when running the server
			set(CMAKE_INSTALL_RPATH "$ORIGIN/../deps:$ORIGIN/../lib")
			set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
			set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

			# use rpath for executables
			# (executable rpath will be used for loading indirect libs, this is needed because boost libs do not set runpath)
			# use newer runpath for shared libs
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags")
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
	OPENSSL_API_COMPAT=0x10100000L
	$<$<BOOL:${ENABLE_CATAPULT_DIAGNOSTICS}>:ENABLE_CATAPULT_DIAGNOSTICS>
	$<$<BOOL:${CATAPULT_DOCKER_TESTS}>:CATAPULT_DOCKER_TESTS>
	$<$<BOOL:${CATAPULT_TEST_DB_URL}>:CATAPULT_TEST_DB_URL="${CATAPULT_TEST_DB_URL}">
)

target_include_directories(build.defaults SYSTEM BEFORE INTERFACE 
	${Boost_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/sdk
	${PROJECT_SOURCE_DIR}/extensions
)
