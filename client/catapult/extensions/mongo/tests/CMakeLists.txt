set(TARGET_NAME tests.catapult.mongo)

subdirectories(ADD)

catapult_test_executable_target(${TARGET_NAME} mongo mappers NOLIB)
target_link_libraries(${TARGET_NAME} 
    ${MongoDB_IMPORTED_TARGETS}
    tests.catapult.test.cache
    catapult.mongo.plugins.transfer # allow transfer to be loaded implicitly
)
