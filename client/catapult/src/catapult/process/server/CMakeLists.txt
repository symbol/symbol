add_target(catapult.server
    EXECUTABLE 
    LINK catapult.local.server catapult.process
    DEPENDENTS server
    SOURCES
)
