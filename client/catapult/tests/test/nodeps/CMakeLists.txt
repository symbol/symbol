add_target(
    tests.catapult.test.nodeps LIBRARY
    LINK_LIBS 
        catapult.crypto
        catapult.utils
        catapult.version
        ${GTest_IMPORTED_TARGETS}
        ${CMAKE_DL_LIBS}
    SOURCES
)
