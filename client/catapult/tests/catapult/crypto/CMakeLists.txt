add_target(
    tests.catapult.crypto TEST
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/external
    LINK_LIBS tests.catapult.test.crypto OpenSSL::SSL
    SOURCES ./
)
