cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.cache)
target_link_libraries(catapult.cache catapult.cache_db catapult.io catapult.model catapult.tree)
