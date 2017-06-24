cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.plugins.coresystem cache handlers observers validators)
target_link_libraries(catapult.plugins.coresystem ${PLUGIN_CATAPULT_LIBS})
