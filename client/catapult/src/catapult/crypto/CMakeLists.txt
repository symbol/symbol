add_target(
    catapult.crypto LIBRARY
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/external
    LINK_LIBS catapult.utils external OpenSSL::SSL
    SOURCES ./
)
