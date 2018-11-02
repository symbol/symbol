cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.mongo.plugins.aggregate)

catapult_mongo_plugin_tests_no_deps(${TARGET_NAME})
