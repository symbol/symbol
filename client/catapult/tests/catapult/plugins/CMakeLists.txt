catapult_test_executable_target(tests.catapult.plugins core NOLIB)
target_link_libraries(tests.catapult.plugins catapult.plugins.transfer) # allow transfer to be loaded implicitly
