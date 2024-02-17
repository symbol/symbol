cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.local.broker)
target_link_libraries(catapult.local.broker catapult.local)
