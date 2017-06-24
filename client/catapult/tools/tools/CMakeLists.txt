cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.tools)
target_link_libraries(catapult.tools ${CORE_CATAPULT_LIBS} catapult.api catapult.net catapult.plugins.transfer)
