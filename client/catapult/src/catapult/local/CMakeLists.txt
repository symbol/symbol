
catapult_library_target(catapult.local)
target_link_libraries(catapult.local catapult.extensions)

add_subdirectory(broker)
add_subdirectory(importer)
add_subdirectory(recovery)
add_subdirectory(server)
