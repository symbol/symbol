subdirectories(ADD BASE . DIRS blockhashes)

catapult_define_tool(nemgen)
target_link_libraries(
	catapult.tools.nemgen
	catapult.tools.nemgen.blockhashes
	catapult.tools.plugins
	catapult.finalization
	catapult.plugins.coresystem
	catapult.plugins.mosaic
	catapult.plugins.namespace
	catapult.plugins.transfer
)
