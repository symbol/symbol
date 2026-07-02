cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.process)
target_link_libraries(catapult.process catapult.version)
catapult_add_openssl_dependencies(catapult.process)

add_subdirectory(broker)
add_subdirectory(importer)
add_subdirectory(recovery)
add_subdirectory(server)

install(TARGETS catapult.broker catapult.importer catapult.recovery catapult.server)
