add_target(
    catapult.extensions LIBRARY
    LINK_LIBS catapult.chain catapult.net catapult.plugins catapult.subscribers
    SOURCES ./
)
