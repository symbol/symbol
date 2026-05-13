add_target(catapult.process
    LIBRARY
    LINK catapult.version
    SOURCES
)

add_subdirs()

install(TARGETS catapult.broker catapult.importer catapult.recovery catapult.server)
