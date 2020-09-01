cmake_minimum_required(VERSION 3.14)

catapult_define_extension_src(finalization api chain handlers io ionet model)
target_link_libraries(catapult.finalization catapult.consumers catapult.crypto_voting)
