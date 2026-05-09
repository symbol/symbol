catapult_library_target(tests.catapult.test.mongo mocks)
target_link_libraries(
    tests.catapult.test.mongo 
    ${MongoDB_IMPORTED_TARGETS}
    tests.catapult.test.core 
    ${GTest_IMPORTED_TARGETS}
)
