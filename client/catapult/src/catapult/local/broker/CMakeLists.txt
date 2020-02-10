cmake_minimum_required(VERSION 3.14)

catapult_library_target(catapult.local.broker)
target_link_libraries(catapult.local.broker catapult.local)
