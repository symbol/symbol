cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME tests.catapult.mongo.plugins.aggregate)

catapult_mongo_plugin_tests(${TARGET_NAME})
