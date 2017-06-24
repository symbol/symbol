cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.plugins.test.mongo.multisig)

catapult_library_target(${TARGET_NAME})
catapult_add_mongo_dependencies(${TARGET_NAME})
