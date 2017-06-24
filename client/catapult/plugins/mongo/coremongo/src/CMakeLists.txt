cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME catapult.plugins.mongo.coremongo)

catapult_library_target(${TARGET_NAME} mappers storages)
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.plugins)
