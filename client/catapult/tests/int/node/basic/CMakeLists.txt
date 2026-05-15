add_target(
    tests.catapult.int.node TEST
    NO_DERIVED_LIB
    LINK_LIBS tests.catapult.int.node.test tests.catapult.test.nemesis
    SOURCES ./
)
set_property(TEST ${TARGET_NAME} PROPERTY LABELS Integration)
