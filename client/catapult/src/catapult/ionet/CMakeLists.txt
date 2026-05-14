add_target(
    catapult.ionet LIBRARY
    LINK_LIBS catapult.model catapult.thread OpenSSL::SSL
    SOURCES ./
)
