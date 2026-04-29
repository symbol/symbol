### find and set zeromq includes and libraries
function(catapult_add_zeromq_dependencies TARGET_NAME)
	target_link_libraries(${TARGET_NAME} cppzmq)
endfunction()

catapult_define_extension(zeromq)
catapult_add_zeromq_dependencies(extension.zeromq)
target_link_libraries(extension.zeromq catapult.extensions)
