cmake_minimum_required(VERSION 3.14)

catapult_test_executable_target_no_lib(tests.catapult.cache cache test)
catapult_add_rocksdb_dependencies(tests.catapult.cache)
