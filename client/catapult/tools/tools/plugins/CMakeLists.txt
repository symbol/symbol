set(TARGET_NAME catapult.tools.plugins)

catapult_library_target(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} catapult.plugins)
