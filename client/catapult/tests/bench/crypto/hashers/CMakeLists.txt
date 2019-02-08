cmake_minimum_required(VERSION 3.2)

catapult_bench_executable_target(bench.catapult.crypto.hashers)
target_link_libraries(bench.catapult.crypto.hashers catapult.crypto tests.catapult.test.nodeps)
