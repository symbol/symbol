catapult_test_executable_target_no_lib(tests.catapult.cache cache test)
target_link_libraries(tests.catapult.cache ${RocksDB_IMPORTED_TARGETS})
