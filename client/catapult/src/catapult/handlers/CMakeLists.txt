cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.handlers)
target_link_libraries(catapult.handlers catapult.ionet catapult.model)
