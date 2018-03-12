cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.int.stress)

catapult_int_test_executable_target(${TARGET_NAME} test)
target_link_libraries(${TARGET_NAME} catapult.plugins.hashcache.cache tests.catapult.test.local)

set_property(TEST ${TARGET_NAME} PROPERTY LABELS Stress)

# add dependency on hash cache plugin
include_directories(../../../plugins/services/hashcache)
