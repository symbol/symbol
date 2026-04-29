
catapult_library_target(tests.catapult.test.nodeps)
target_link_libraries(tests.catapult.test.nodeps catapult.crypto catapult.utils catapult.version ${GTest_IMPORTED_TARGETS} ${CMAKE_DL_LIBS})
