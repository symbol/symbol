set(TARGET_NAME catapult.tools.nemgen.blockhashes)

catapult_library_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.chain)
