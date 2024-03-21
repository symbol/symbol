cmake_minimum_required(VERSION 3.23)

catapult_test_executable_target_no_lib(tests.catapult.keylink cache)
target_link_libraries(tests.catapult.keylink catapult.observers catapult.validators)
