add_target(
    tests.catapult.crypto TEST
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/external
    LINK_LIBS crypto OpenSSL::SSL
    SOURCES ./
)

#include_directories(../../../external)
#catapult_test_executable_target(tests.catapult.crypto crypto NOLIB)
#catapult_add_openssl_dependencies(tests.catapult.crypto)
