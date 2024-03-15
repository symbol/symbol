cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.extensions)
target_link_libraries(catapult.extensions catapult.chain catapult.net catapult.plugins catapult.subscribers)
