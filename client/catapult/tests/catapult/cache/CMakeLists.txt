catapult_test_executable_target(tests.catapult.cache cache test NOLIB)
target_link_libraries(tests.catapult.cache ${RocksDB_IMPORTED_TARGETS})
