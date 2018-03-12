cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.coresystem handlers observers validators)
target_link_libraries(catapult.coresystem ${PLUGIN_CATAPULT_LIBS})
