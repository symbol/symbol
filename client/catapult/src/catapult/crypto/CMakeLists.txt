cmake_minimum_required(VERSION 3.2)

include_directories(../../../external)

catapult_library_target(catapult.crypto)

target_link_libraries(catapult.crypto catapult.utils external)
catapult_add_ssl_dependencies(catapult.crypto)
