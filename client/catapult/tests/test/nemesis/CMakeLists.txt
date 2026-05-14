add_target(
	tests.catapult.test.nemesis LIBRARY
	INCLUDE_DIRS 
		${CMAKE_SOURCE_DIR}/plugins/txes/mosaic
		${CMAKE_SOURCE_DIR}/plugins/txes/namespace
	LINK_LIBS 
		tests.catapult.test.local
		catapult.plugins.mosaic
		catapult.plugins.namespace
		catapult.plugins.transfer # shared lib dependencies allow tests to create real transaction plugins
	SOURCES
)
