add_target(
    tests.catapult.test.crypto
    LIBRARY
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/external
    LINK_LIBS tests.catapult.test.nodeps OpenSSL::SSL
    SOURCES ./
)
