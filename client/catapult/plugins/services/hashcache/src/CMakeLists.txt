cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.plugins.hashcache)

# create a cache lib
catapult_library_target(${PLUGIN_BASE_NAME}.cache cache state)
target_link_libraries(${PLUGIN_BASE_NAME}.cache catapult.cache)

# create a deps lib
catapult_library_target(${PLUGIN_BASE_NAME}.deps handlers observers validators)
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS} ${PLUGIN_BASE_NAME}.cache)

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
