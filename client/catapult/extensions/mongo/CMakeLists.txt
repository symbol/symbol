### find and set mongo includes and libraries
function(catapult_add_mongo_dependencies TARGET_NAME)
	target_link_libraries(${TARGET_NAME} $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>)
endfunction()

add_subdirs(./plugins)

catapult_define_extension_no_group(mongo)
catapult_add_mongo_dependencies(extension.mongo)

# add extension to mongo group
add_dependencies(mongo extension.mongo)
