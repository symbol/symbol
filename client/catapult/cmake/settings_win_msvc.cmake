# MSVC specific compiler settings
# MSVC (Visual Studio 2019 16.8) is the minimum supported version
# We support only cl actually, clang-cl is not (yet) supported

if (MSVC_VERSION LESS 1928)
	message(FATAL_ERROR "MSVC version must be at least 1928 (Visual Studio 2019 16.8)\nFound version ${MSVC_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(FATAL_ERROR "Clang-cl is not (yet) supported.\nPlease use MSVC's cl compiler.")
endif ()

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
	$<$<COMPILE_LANGUAGE:CXX>:/GA>												# Optimizes for Windows applications
	$<$<COMPILE_LANGUAGE:CXX>:/EHsc>											# Enable C++ exceptions, but not SEH exceptions (which are not used in catapult)
	
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<BOOL:${ENABLE_HARDENING}>,$<CONFIG:Release,RelWithDebInfo>>>:/sdl>			# Enable additional security checks

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:/MDd>						# Compiles to create a debug multithreaded DLL, by using MSVCRTD.lib.
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:/MD>				# Compiles to create a multithreaded DLL, by using MSVCRT.lib.

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<STREQUAL:${ARCHITECTURE_NAME},>>>:/arch:${ARCHITECTURE_NAME}>

	# Enable Named Return Value Optimization for Debug builds when supported by MSVC (VS 2022 17.4+)
	# Note !! This is explicitly enabled for Debug builds only as for other build types, the optimization is already enabled by default.
	# Rationale : Many tests rely on the fact NRVO is applied. If we don't do this running tests in Debug mode will cause a significant
	# amount of errors. Due to this running tests in Debug mode for Visual Studio versions less than 17.4 will be very difficult and error prone.
	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>,$<VERSION_GREATER_EQUAL:${MSVC_VERSION},1934>>:/Zc:nrvo>

	$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/MP${PARALLEL_BUILDS}>		# Uses multiple processes

)

target_link_options(build.defaults INTERFACE
	$<$<CONFIG:Debug>:/DEBUG>
	$<$<CONFIG:RelWithDebInfo>:/DEBUG>
	$<$<CONFIG:RelWithDebInfo>:/INCREMENTAL:NO>
	$<$<CONFIG:RelWithDebInfo>:/OPT:REF>
)

if (CCACHE_EXE AND USE_CCACHE_ON_WINDOWS)
	set(_msvc_debug_flag "/Z7")
else()
	set(_msvc_debug_flag "/Zi")
endif()

target_compile_options(build.defaults INTERFACE $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:${_msvc_debug_flag}>)
