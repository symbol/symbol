cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.mongo.plugins.multisig)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/multisig)

# create a deps lib
catapult_library_target(${PLUGIN_BASE_NAME}.deps mappers storages)
catapult_add_mongo_dependencies(${PLUGIN_BASE_NAME}.deps)

# multisig.deps is needed since the mongo multisig cache storage needs the multisig cache
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS} catapult.plugins.multisig.deps)

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
