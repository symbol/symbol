cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME catapult.plugins.mongo.aggregate)

catapult_shared_library_target(${TARGET_NAME})
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} ${PLUGIN_CATAPULT_LIBS} catapult.plugins.aggregate)
