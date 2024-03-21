cmake_minimum_required(VERSION 3.23)

add_subdirectory(plugins)

catapult_library_target(catapult.tools)
target_link_libraries(catapult.tools ${CORE_CATAPULT_LIBS} catapult.api catapult.config catapult.net catapult.sdk catapult.version)
