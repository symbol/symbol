add_target(
    tests.catapult.int.node.stress TEST
    NO_DERIVED_LIB
    LINK_LIBS tests.catapult.int.node.test tests.catapult.test.nemesis
    SOURCES ./ ./test
)
set_property(TEST ${TARGET_NAME} PROPERTY LABELS Integration Stress)
