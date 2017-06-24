cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.plugins.multisig)

# create a deps lib (notice that model and state are empty)
catapult_library_target(${PLUGIN_BASE_NAME}.deps cache config model observers state validators)
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS})

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
