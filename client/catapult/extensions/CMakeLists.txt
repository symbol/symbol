include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src)

function(catapult_define_extension_no_group EXTENSION_NAME)
	subdirectories(ADD DIRS src tests)
	catapult_shared_library_target(extension.${EXTENSION_NAME})
	target_link_libraries(extension.${EXTENSION_NAME} catapult.${EXTENSION_NAME})
	set_property(TARGET extension.${EXTENSION_NAME} PROPERTY FOLDER "extensions")
endfunction()

function(catapult_define_extension EXTENSION_NAME)
	catapult_define_extension_no_group(${EXTENSION_NAME})
	add_dependencies(extensions extension.${EXTENSION_NAME})
endfunction()

function(catapult_define_extension_src EXTENSION_NAME)
	catapult_library_target(catapult.${EXTENSION_NAME} ${ARGN})
	target_link_libraries(catapult.${EXTENSION_NAME} catapult.extensions catapult.handlers catapult.net)
	set_property(TARGET catapult.${EXTENSION_NAME} PROPERTY FOLDER "extensions")
endfunction()

function(catapult_define_extension_test EXTENSION_NAME)
	catapult_test_executable_target(tests.catapult.${EXTENSION_NAME} local ${ARGN})
	set_property(TARGET tests.catapult.${EXTENSION_NAME} PROPERTY FOLDER "extensions")
endfunction()

subdirectories(ADD)
