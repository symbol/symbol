add_target(catapult.broker 
    EXECUTABLE 
    LINK catapult.local.broker catapult.process
    DEPENDENTS server
    SOURCES
)
