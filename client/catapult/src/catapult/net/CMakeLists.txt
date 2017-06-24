cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.net)
target_link_libraries(catapult.net catapult.ionet catapult.thread)
