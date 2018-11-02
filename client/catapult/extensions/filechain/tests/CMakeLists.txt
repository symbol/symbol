cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME tests.catapult.filechain)

catapult_define_extension_test(filechain test)
target_link_libraries(${TARGET_NAME} tests.catapult.test.nemesis)
