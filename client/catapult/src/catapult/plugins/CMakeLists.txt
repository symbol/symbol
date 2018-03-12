cmake_minimum_required(VERSION 3.2)

# statically link against plugins so that they're available in PluginLoader
catapult_library_target(catapult.plugins)
target_link_libraries(catapult.plugins catapult.coresystem ${CMAKE_DL_LIBS})
