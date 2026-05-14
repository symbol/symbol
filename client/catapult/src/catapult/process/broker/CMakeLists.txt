add_target(catapult.broker 
    EXECUTABLE 
    WITH_INSTALL
    LINK_LIBS catapult.local.broker catapult.process
    DEPENDENTS server
    SOURCES ./
)
