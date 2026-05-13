add_target(catapult.recovery
    EXECUTABLE 
    LINK catapult.local.recovery catapult.process
    DEPENDENTS server
    SOURCES
)
