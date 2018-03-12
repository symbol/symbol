cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.mongo.plugins.lock)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/lock)

# create a deps lib
catapult_library_target(${PLUGIN_BASE_NAME}.deps mappers storages)
catapult_add_mongo_dependencies(${PLUGIN_BASE_NAME}.deps)

# lock.deps is needed since the mongo lock info cache storage needs the lock info cache
target_link_libraries(${PLUGIN_BASE_NAME}.deps ${PLUGIN_CATAPULT_LIBS} catapult.plugins.lock.deps)

# create a plugin dll
catapult_shared_library_target(${PLUGIN_BASE_NAME} plugins)
target_link_libraries(${PLUGIN_BASE_NAME} ${PLUGIN_BASE_NAME}.deps)
