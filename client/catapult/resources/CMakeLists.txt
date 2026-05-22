file(GLOB _sources "*.properties" "*.json")
foreach(_item ${_sources})
	set_source_files_properties(${_item} PROPERTIES HEADER_FILE_ONLY TRUE)
endforeach()
add_target(
	resources CUSTOM
	SOURCES ${_sources}
)
