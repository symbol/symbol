add_target(
    tests.catapult.test.cache LIBRARY
    LINK_LIBS tests.catapult.test.core catapult.cache catapult.cache_core catapult.cache_tx
    SOURCES ./
)
