cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.plugins.blockdifficultycache cache observers)
target_link_libraries(catapult.plugins.blockdifficultycache ${PLUGIN_CATAPULT_LIBS})
