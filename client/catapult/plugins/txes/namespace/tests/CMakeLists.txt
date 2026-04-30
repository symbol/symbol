subdirectories(ADD BASE . DIRS test)

set(TARGET_NAME tests.catapult.plugins.namespace)

catapult_tx_plugin_tests(${TARGET_NAME})
