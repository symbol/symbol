add_target(
    catapult.cache LIBRARY
    LINK_LIBS catapult.cache_db catapult.io catapult.model catapult.tree
    SOURCES
)
