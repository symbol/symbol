cmake_minimum_required(VERSION 3.23)

include_directories(../../../external)

catapult_test_executable_target_no_lib(tests.catapult.crypto crypto)
catapult_add_openssl_dependencies(tests.catapult.crypto)
