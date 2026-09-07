cmake_minimum_required(VERSION 3.23)

set(TARGET_NAME catapult.tools.nemgen.blockhashes)

catapult_library_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.chain)

# library has chain dependency so it must build against src (not the published sdk headers, which do not include it);
# its public headers only include published headers, so tools can still consume it while building against the sdk
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
