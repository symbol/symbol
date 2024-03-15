cmake_minimum_required(VERSION 3.23)

catapult_define_extension_src(syncsource)
target_link_libraries(catapult.syncsource catapult.io)
