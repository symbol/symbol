add_target(
    catapult.validators LIBRARY
    LINK_LIBS catapult.cache catapult.thread
    SOURCES ./
)
