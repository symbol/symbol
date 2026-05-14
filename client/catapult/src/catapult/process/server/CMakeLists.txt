add_target(catapult.server
    EXECUTABLE 
    WITH_INSTALL
    LINK_LIBS catapult.local.server catapult.process
    DEPENDENTS server
    SOURCES
)
