cmake_minimum_required(VERSION 3.2)

catapult_test_executable_target(tests.catapult.local.p2p local utils)

# add dependency on hash cache plugin
include_directories(../../../../plugins/services/hashcache)
