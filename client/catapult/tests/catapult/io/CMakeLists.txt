cmake_minimum_required(VERSION 3.23)

# use no_lib to avoid 'duplicate libraries' warning because tests.catapult.test.core pulls in catapult.io
catapult_test_executable_target_no_lib(tests.catapult.io core test)
