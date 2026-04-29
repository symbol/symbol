catapult_test_executable_target(tests.catapult.keylink cache NOLIB)
target_link_libraries(tests.catapult.keylink catapult.observers catapult.validators)
