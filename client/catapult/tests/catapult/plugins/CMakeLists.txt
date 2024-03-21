cmake_minimum_required(VERSION 3.23)

catapult_test_executable_target_no_lib(tests.catapult.plugins core)
target_link_libraries(tests.catapult.plugins catapult.plugins.transfer) # allow transfer to be loaded implicitly
