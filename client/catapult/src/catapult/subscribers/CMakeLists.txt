add_target(
    catapult.subscribers LIBRARY
    LINK_LIBS catapult.cache catapult.cache_tx catapult.config catapult.validators
    SOURCES
)
