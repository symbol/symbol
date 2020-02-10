cmake_minimum_required(VERSION 3.14)

message("--- locating zeromq dependencies ---")
# ZeroMQ does not support CMake for now, pkg-config fallback will be done by cppzmqConfig.cmake
find_package(cppzmq REQUIRED)

# ZeroMQ_LIBRARY might not always be set, but project should link anyway
if(ZeroMQ_LIBRARY)
	message("zeromq    lib: ${ZeroMQ_LIBRARY}")
endif()

message("cppzmq    inc: ${cppzmq_INCLUDE_DIR}")

### find and set zeromq includes and libraries
function(catapult_add_zeromq_dependencies TARGET_NAME)
	# libzmq will be included by cppzmq
	target_link_libraries(${TARGET_NAME} cppzmq)
endfunction()

catapult_define_extension(zeromq)
catapult_add_zeromq_dependencies(extension.zeromq)
target_link_libraries(extension.zeromq catapult.extensions)
