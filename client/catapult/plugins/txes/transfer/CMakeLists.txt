cmake_minimum_required(VERSION 3.23)

set(PLUGIN_DEPS_FOLDERS config model observers validators)

include_directories(.)
add_subdirectory(src)
catapult_add_tests_subdirectory(tests)
