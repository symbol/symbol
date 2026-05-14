add_target(catapult.importer
    EXECUTABLE 
    WITH_INSTALL
    LINK_LIBS catapult.local.importer catapult.process
    DEPENDENTS server
    SOURCES
)
