add_subdirectory(blockhashes)

add_target(nemgen TOOL
	LINK_LIBS
	catapult.tools.nemgen.blockhashes
	catapult.tools.plugins
	catapult.finalization
	catapult.plugins.coresystem
	catapult.plugins.mosaic
	catapult.plugins.namespace
	catapult.plugins.transfer
	SOURCES ./
)
