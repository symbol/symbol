cmake_minimum_required(VERSION 3.23)

catapult_define_extension_src(hashcache)

# add dependency on hash cache plugin
target_link_libraries(catapult.hashcache catapult.plugins.hashcache.cache)
