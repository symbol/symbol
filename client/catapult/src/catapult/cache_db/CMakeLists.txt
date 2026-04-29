set(TARGET_NAME catapult.cache_db)

catapult_library_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.config catapult.tree ${RocksDB_IMPORTED_TARGETS})
