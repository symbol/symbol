cmake_minimum_required(VERSION 3.23)

catapult_library_target(catapult.local.importer)
target_link_libraries(catapult.local.importer catapult.local.recovery catapult.local.server)
