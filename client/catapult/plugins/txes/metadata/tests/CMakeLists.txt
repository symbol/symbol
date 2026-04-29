subdirectories(ADD BASE . DIRS test VERBOSE)

set(TARGET_NAME tests.catapult.plugins.metadata)

catapult_tx_plugin_tests(${TARGET_NAME})
