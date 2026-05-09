set(TARGET_NAME tests.catapult.int.mongo.plugins.restrictionmosaic)

catapult_int_test_executable_target(${TARGET_NAME} storages)
target_link_libraries(${TARGET_NAME}
	${MongoDB_IMPORTED_TARGETS}
	tests.catapult.test.cache
	tests.catapult.test.mongo
	tests.catapult.test.mongo.plugins.restrictionmosaic
	catapult.mongo.plugins.restrictionmosaic)
