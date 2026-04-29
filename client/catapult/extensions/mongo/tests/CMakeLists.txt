set(TARGET_NAME tests.catapult.mongo)

subdirectories(ADD BASE . VERBOSE)

catapult_test_executable_target(${TARGET_NAME} mongo mappers NOLIB)
catapult_add_mongo_dependencies(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} tests.catapult.test.cache)
target_link_libraries(${TARGET_NAME} catapult.mongo.plugins.transfer) # allow transfer to be loaded implicitly
