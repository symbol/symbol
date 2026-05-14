add_target(
    tests.catapult.plugins.lockshared LIBRARY
    TYPE INTERFACE
    SOURCES ./ ./cache ./observers ./state ./validators ./test ./validators
)
