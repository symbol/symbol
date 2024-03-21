cmake_minimum_required(VERSION 3.23)

catapult_library_target(tests.catapult.int.node.test)
target_link_libraries(tests.catapult.int.node.test catapult.plugins.coresystem tests.catapult.test.local)
