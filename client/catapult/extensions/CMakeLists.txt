function(catapult_define_extension_src EXTENSION_NAME)
	catapult_library_target(catapult.${EXTENSION_NAME} ${ARGN})
	target_link_libraries(catapult.${EXTENSION_NAME} catapult.extensions catapult.handlers catapult.net)
	set_property(TARGET catapult.${EXTENSION_NAME} PROPERTY FOLDER "extensions")
endfunction()

function(catapult_define_extension_test EXTENSION_NAME)
	set(_source_dirs ./)
	foreach(_relative_source IN LISTS ARGN)
		list(APPEND _source_dirs "./${_relative_source}")
	endforeach()

	add_target(tests.catapult.${EXTENSION_NAME} TEST
		LINK_LIBS tests.catapult.test.local
		SOURCES ${_source_dirs}
	)
	set_property(TARGET tests.catapult.${EXTENSION_NAME} PROPERTY FOLDER "extensions")
endfunction()

add_subdirs()
