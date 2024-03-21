cmake_minimum_required(VERSION 3.23)

catapult_test_executable_target_no_lib(tests.catapult.ionet net)
catapult_add_openssl_dependencies(tests.catapult.ionet)
