# MSVC specific compiler settings
# MSVC (Visual Studio 2019 16.8) is the minimum supported version
# We support both cl and clang-cl, but both require the same minimum version of Visual Studio

if (MSVC_VERSION LESS_EQUAL 1928)
	message(FATAL_ERROR "MSVC version must be at least 1928 (Visual Studio 2019 16.8)\nFound version ${MSVC_VERSION}")
endif ()

# Set a flag we're using cl or clang-cl, which is used in some conditional settings below
set(CACHE MSVC_EXTENDED TYPE BOOL FORCE VALUE TRUE)

target_compile_definitions(build.defaults INTERFACE 
	_WIN32_WINNT=0x0A00											# Min Windows 10
	VC_EXTRALEAN												# Process windows headers faster ...
	WIN32_LEAN_AND_MEAN											# ... and prevent winsock mismatch with Boost's
	NOMINMAX													# Prevent MSVC to tamper with std::min/std::max
	PSAPI_VERSION=2												# For process info
	BOOST_ALL_NO_LIB											# explicitly disable linking against static boost libs
	_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING		# mongo cxx view inherits std::iterator
	_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING			# boost asio associated_allocator
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:_SCL_SECURE_NO_WARNINGS>
)

target_compile_options(build.defaults INTERFACE
	$<$<COMPILE_LANGUAGE:CXX>:/permissive->										# enable standard conformance mode
	$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>									# enable correct __cplusplus macro
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:/Zc:nrvo>					# enable Named Return Value Optimization for Debug builds
	$<$<COMPILE_LANGUAGE:CXX>:/W4>												# set warning level to 4
	$<$<COMPILE_LANGUAGE:CXX>:/WX>												# treat warnings as errors
	$<$<COMPILE_LANGUAGE:CXX>:/w44287>											# 'operator' : unsigned/negative constant mismatch
	$<$<COMPILE_LANGUAGE:CXX>:/w44388>											# 'token' : signed/unsigned mismatch
	$<$<COMPILE_LANGUAGE:CXX>:/wd4127>											# Silence warnings about "conditional expression is constant" (abseil mainly)
	$<$<COMPILE_LANGUAGE:CXX>:/wd4068>											# Silence warning C4068: unknown pragma
	$<$<COMPILE_LANGUAGE:CXX>:/wd4324>											# Silence warning C4324: 'xxx': structure was padded due to alignment specifier
	$<$<COMPILE_LANGUAGE:CXX>:/wd4701>											# Silence warning C4701: potentially uninitialized local variable used
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:/wd4702>			# Silence warning C4702: unreachable code
	$<$<COMPILE_LANGUAGE:CXX>:/wd4714>											# Silence warning C4714: _forceinline not inlined
	$<$<COMPILE_LANGUAGE:CXX>:/wd5030>											# Silence warning C5030: unknown gnu/clang attribute
	$<$<COMPILE_LANGUAGE:CXX>:/MP${PARALLEL_BUILDS}>							# Uses multiple processes
	$<$<COMPILE_LANGUAGE:CXX>:/GA>												# Optimizes for Windows applications
	$<$<COMPILE_LANGUAGE:CXX>:/EHsc>											# Enable C++ exceptions, but not SEH exceptions (which are not used in catapult)
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${ENABLE_HARDENING}>>:/sdl>			# Enable additional security checks
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:/MDd>						# Compiles to create a debug multithreaded DLL, by using MSVCRTD.lib.
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:/MD>				# Compiles to create a multithreaded DLL, by using MSVCRT.lib.

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<STREQUAL:${ARCHITECTURE_NAME},>>>:/arch:${ARCHITECTURE_NAME}>

)

target_link_options(build.defaults INTERFACE
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:/DEBUG>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:/DEBUG>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:/INCREMENTAL:NO>
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:/OPT:REF>
)

if (CCACHE_EXE AND USE_CCACHE_ON_WINDOWS)
	set(_msvc_debug_flag "/Z7")
else()
	set(_msvc_debug_flag "/Zi")
endif()

target_compile_options(build.defaults INTERFACE $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:${_msvc_debug_flag}>)
