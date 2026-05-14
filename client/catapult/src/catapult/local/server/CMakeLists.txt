add_target(
    catapult.local.server LIBRARY
    LINK_LIBS catapult.local catapult.handlers catapult.net
    SOURCES ./
)
