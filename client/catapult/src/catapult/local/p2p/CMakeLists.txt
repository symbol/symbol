cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.local.p2p)
target_link_libraries(catapult.local.p2p catapult.local)

# add dependency on hash cache plugin (required for p2p nodes)
target_link_libraries(catapult.local.p2p catapult.plugins.hashcache.cache)
