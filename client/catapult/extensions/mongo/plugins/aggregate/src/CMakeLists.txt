cmake_minimum_required(VERSION 3.14)

set(PLUGIN_BASE_NAME catapult.mongo.plugins.aggregate)

catapult_mongo_plugin_src_no_deps(${PLUGIN_BASE_NAME})
target_link_libraries(${PLUGIN_BASE_NAME} catapult.plugins.aggregate)
