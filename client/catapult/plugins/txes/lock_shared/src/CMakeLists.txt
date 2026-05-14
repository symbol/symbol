add_target(
    catapult.plugins.lockshared LIBRARY
    TYPE INTERFACE
    SOURCES ./cache ./model ./observers ./state ./validators
)
