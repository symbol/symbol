subdirectories(ADD DIRS test)

set(TARGET_NAME tests.catapult.plugins.locksecret)

catapult_tx_plugin_tests(${TARGET_NAME})
