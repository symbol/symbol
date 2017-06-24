cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME catapult.api.server)

catapult_executable(${TARGET_NAME})
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.servermain catapult.local.api catapult.plugins.mongo.coremongo)
catapult_target(${TARGET_NAME})
