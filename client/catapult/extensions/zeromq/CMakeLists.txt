### find and set zeromq includes and libraries
function(catapult_add_zeromq_dependencies TARGET_NAME)
	target_link_libraries(${TARGET_NAME} cppzmq)
endfunction()

add_subdirs()
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME _extension_name)
add_target(
	${_extension_name} EXTENSION SHARED
	LINK_LIBS 
		cppzmq
		catapult.${_extension_name}
		catapult.extensions
	FOLDER "extensions"
	SOURCES ./
)
