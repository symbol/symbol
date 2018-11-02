cmake_minimum_required(VERSION 3.2)

set(PLUGIN_BASE_NAME catapult.mongo.plugins.property)

include_directories(${PROJECT_SOURCE_DIR}/plugins/txes/property)
catapult_mongo_plugin_src_with_deps(${PLUGIN_BASE_NAME} mappers storages)

# property.deps is needed because the mongo property cache storage needs the property cache
target_link_libraries(${PLUGIN_BASE_NAME}.deps catapult.plugins.property.deps)
