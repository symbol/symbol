cmake_minimum_required(VERSION 3.23)

add_subdirectory(plugins)

catapult_library_target(catapult.tools)
target_include_directories(catapult.tools PRIVATE ${PROJECT_SOURCE_DIR}/tools ${CMAKE_BINARY_DIR}/inc)
add_dependencies(catapult.tools catapult_sdk_publish)
target_link_libraries(catapult.tools ${CORE_CATAPULT_LIBS} catapult.api catapult.config catapult.net catapult.sdk catapult.version)
