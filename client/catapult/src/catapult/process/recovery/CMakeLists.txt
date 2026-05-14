add_target(catapult.recovery
    EXECUTABLE 
    WITH_INSTALL
    LINK_LIBS catapult.local.recovery catapult.process
    DEPENDENTS server
    SOURCES ./
)
