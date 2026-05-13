add_target(catapult.importer
    EXECUTABLE 
    LINK catapult.local.importer catapult.process
    DEPENDENTS server
    SOURCES
)
