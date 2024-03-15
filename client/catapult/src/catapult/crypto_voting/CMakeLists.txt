cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.crypto_voting)
target_link_libraries(catapult.crypto_voting catapult.crypto)
