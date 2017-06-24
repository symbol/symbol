cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.plugins.aggregate)

# create an sdk lib
catapult_library_target(${PLUGIN_BASE_NAME}.sdk model)
target_link_libraries(${PLUGIN_BASE_NAME}.sdk catapult.model)

# create a deps lib
catapult_library_target(${PLUGIN_BASE_NAME}.deps config validators)
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS} ${PLUGIN_BASE_NAME}.sdk)

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_CATAPULT_LIBS} ${PLUGIN_BASE_NAME}.deps)
