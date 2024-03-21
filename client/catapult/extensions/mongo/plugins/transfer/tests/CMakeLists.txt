cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME tests.catapult.mongo.plugins.transfer)

catapult_mongo_plugin_tests_no_deps(${TARGET_NAME})
