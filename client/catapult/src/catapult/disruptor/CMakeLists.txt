cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.disruptor)
target_link_libraries(catapult.disruptor catapult.model)
