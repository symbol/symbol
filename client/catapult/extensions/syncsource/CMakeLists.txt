add_subdirs()
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME _extension_name)
add_target(
	${_extension_name} EXTENSION SHARED
	LINK_LIBS catapult.${_extension_name}
	FOLDER "extensions"
	SOURCES ./
)
