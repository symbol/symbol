add_target(
    catapult.cache_db LIBRARY
    LINK_LIBS catapult.config catapult.tree ${RocksDB_IMPORTED_TARGETS}
    SOURCES ./
)
