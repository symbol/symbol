cmake_minimum_required(VERSION 3.23)

set(PLUGIN_BASE_NAME catapult.plugins.coresystem)

# create a deps lib
catapult_library_target(${PLUGIN_BASE_NAME}.deps importance observers validators)
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS})

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)

add_dependencies(plugins ${PLUGIN_BASE_NAME})
