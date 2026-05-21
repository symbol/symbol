### find and set mongo includes and libraries
function(catapult_add_mongo_dependencies TARGET_NAME)
	target_link_libraries(${TARGET_NAME} $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>)
endfunction()

# used to define a catapult mongo int test executable by combining catapult_test_executable and catapult_target
function(catapult_int_test_executable_target TARGET_NAME)

	add_target(${TARGET_NAME} TEST
		WNO_DERIVED_LIB
		LABELS Database
		SOURCES
			./ 
			${ARGN}
	)

endfunction()

add_subdirectory(plugins)

catapult_define_extension_no_group(mongo)
catapult_add_mongo_dependencies(extension.mongo)

# add extension to mongo group
add_dependencies(mongo extension.mongo)
