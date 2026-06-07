# Sanitize project toolchain file if any.
# IMPORTANT ! This MUST be done before any project() or find_package() calls
# as after any of the above the CMAKE_TOOLCHAIN_FILE is locked in.

function(_validate_vcpkg_builtin_baseline TOOLCHAIN_FILE_PATH)
	if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vcpkg.json")
		return()
	endif()

	file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" _vcpkg_manifest)
	string(REGEX MATCH "\"builtin-baseline\"[ \t\r\n]*:[ \t\r\n]*\"([0-9a-fA-F]+)\"" _baseline_match "${_vcpkg_manifest}")
	set(_builtin_baseline "${CMAKE_MATCH_1}")
	if(_builtin_baseline STREQUAL "")
		return()
	endif()

	get_filename_component(_buildsystems_dir "${TOOLCHAIN_FILE_PATH}" DIRECTORY)
	get_filename_component(_scripts_dir "${_buildsystems_dir}" DIRECTORY)
	get_filename_component(_vcpkg_root "${_scripts_dir}" DIRECTORY)
	if(NOT EXISTS "${_vcpkg_root}/.git")
		return()
	endif()

	set(_git_executable "${GIT_EXECUTABLE}")
	if(NOT _git_executable)
		find_program(_git_executable NAMES git)
	endif()
	if(NOT _git_executable)
		return()
	endif()

	execute_process(
		COMMAND "${_git_executable}" -C "${_vcpkg_root}" cat-file -e "${_builtin_baseline}^{commit}"
		RESULT_VARIABLE _baseline_result
		OUTPUT_QUIET
		ERROR_QUIET
	)
	if(NOT _baseline_result EQUAL 0)
		message(FATAL_ERROR
			"The vcpkg clone at '${_vcpkg_root}' does not contain builtin-baseline '${_builtin_baseline}' from '${CMAKE_SOURCE_DIR}/vcpkg.json'.\n"
			"Update that clone and rerun configure, for example:\n"
			"  git -C \"${_vcpkg_root}\" pull --ff-only\n"
			"If the baseline is still missing after that, fetch the latest refs for the clone and retry."
		)
	endif()
endfunction()

if(DEFINED CMAKE_TOOLCHAIN_FILE)

	get_filename_component(_toolchain_abs "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
	get_filename_component(_toolchain_leaf "${_toolchain_abs}" NAME)

	# Lower case comparison
	string(TOLOWER "${_toolchain_leaf}" _toolchain_leaf_lower)
	if(_toolchain_leaf_lower STREQUAL "vcpkg.cmake")
		set(USE_VCPKG ON CACHE BOOL "Use vcpkg")
		if(WIN32)
			set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
		elseif(LINUX)
			set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
		endif()
		if(NOT DEFINED VCPKG_OVERLAY_PORTS OR VCPKG_OVERLAY_PORTS STREQUAL "")
			set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg-ports" CACHE STRING "Vcpkg overlay ports")
		endif()
		_validate_vcpkg_builtin_baseline("${_toolchain_abs}")
	elseif(_toolchain_leaf_lower MATCHES "^conan.*\.cmake$")
		set(USE_CONAN ON CACHE BOOL "Use Conan")
	else()
		message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE must be either a vcpkg.cmake or a Conan-generated cmake file. Provided: ${CMAKE_TOOLCHAIN_FILE}")
	endif()

	unset(_toolchain_abs)
	unset(_toolchain_leaf)
	unset(_toolchain_leaf_lower)

else()

	set(USE_METAL ON CACHE BOOL "Use Metal")

endif()

set(CONAN_SYSTEM_INCLUDES $<BOOL:${USE_CONAN}>)

### set general cmake settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
