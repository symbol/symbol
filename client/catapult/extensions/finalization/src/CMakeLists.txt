cmake_minimum_required(VERSION 3.14)

catapult_define_extension_src(finalization handlers io model)
target_link_libraries(catapult.finalization catapult.crypto_voting)
