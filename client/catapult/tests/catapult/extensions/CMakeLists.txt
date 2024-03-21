cmake_minimum_required(VERSION 3.23)

catapult_test_executable_target_no_lib(tests.catapult.extensions nemesis)
target_link_libraries(tests.catapult.extensions catapult.plugins.coresystem.deps)
catapult_add_openssl_dependencies(tests.catapult.extensions)
