cmake_minimum_required(VERSION 3.23)

add_subdirectory(test)

set(TARGET_NAME tests.catapult.plugins.mosaic)

catapult_tx_plugin_tests(${TARGET_NAME})
