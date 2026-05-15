add_target(
	tests.catapult.int.stress TEST
	NO_DERIVED_LIB
	INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/plugins/services/hashcache
	LINK_LIBS 
		catapult.harvesting
		catapult.plugins.hashcache
		tests.catapult.test.nemesis
	SOURCES ./ ./test
)
set_property(TEST ${TARGET_NAME} PROPERTY LABELS Integration Stress)
