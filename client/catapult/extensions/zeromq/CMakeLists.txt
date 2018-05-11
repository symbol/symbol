cmake_minimum_required(VERSION 3.2)

message("--- locating zeromq dependencies ---")
find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)

message("  zeromq dirs: ${ZeroMQ_INCLUDE_DIR}")
message("   cppmq dirs: ${cppzmq_INCLUDE_DIR}")

### find and set zeromq includes and libraries
function(catapult_add_zeromq_dependencies TARGET_NAME)
	include_directories(SYSTEM ${ZeroMQ_INCLUDE_DIR})
	target_link_libraries(${TARGET_NAME} libzmq)
endfunction()

catapult_define_extension(zeromq)
catapult_add_zeromq_dependencies(extension.zeromq)
target_link_libraries(extension.zeromq catapult.extensions)
