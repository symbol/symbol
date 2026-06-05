glob_subdirs(_all_tools RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
foreach(_item IN LISTS _all_tools)

    string(REPLACE "_" "" _name ${_item})
    cmake_path(APPEND _tool_path "${CMAKE_CURRENT_SOURCE_DIR}" "${_name}")
    cmake_path(APPEND _tool_own_cmake_list "${_tool_path}" "CMakeLists.txt")
    if(EXISTS "${_tool_own_cmake_list}")
		# A custom CMakeLists.txt exists for this tool, 
		# so we defer to it and just add the subdir
		add_subdirs("${_tool_path}")
		continue()
	endif()

	add_target(${_name} TOOL SOURCES ${_tool_path})

endforeach()
