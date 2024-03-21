cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.validators)
target_link_libraries(catapult.validators catapult.cache catapult.thread)
