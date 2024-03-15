cmake_minimum_required(VERSION 3.23)

catapult_library_target(bench.catapult.bench.nodeps)
target_link_libraries(bench.catapult.bench.nodeps benchmark::benchmark)  # this also sets up benchmark include directories
target_link_libraries(bench.catapult.bench.nodeps catapult.utils catapult.version)
