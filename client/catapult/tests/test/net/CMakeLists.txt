add_target(
    tests.catapult.test.net LIBRARY
    LINK_LIBS 
        tests.catapult.test.core
        tests.catapult.test.crypto
        catapult.net
        OpenSSL::SSL
    SOURCES ./ ./mocks
)
