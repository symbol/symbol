cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.plugins.namespace)

# create an sdk lib
catapult_library_target(${PLUGIN_BASE_NAME}.sdk model state)
target_link_libraries(${PLUGIN_BASE_NAME}.sdk catapult.crypto)

# create a deps lib (namespace plugin requires account cache)
catapult_library_target(${PLUGIN_BASE_NAME}.deps cache config handlers observers validators)
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS} ${PLUGIN_BASE_NAME}.sdk catapult.plugins.coresystem)

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
