cmake_minimum_required(VERSION 3.23)

add_subdirectory(blockhashes)

catapult_define_tool(nemgen)
target_link_libraries(catapult.tools.nemgen
	catapult.tools.nemgen.blockhashes
	catapult.tools.plugins
	catapult.finalization
	catapult.plugins.coresystem
	catapult.plugins.mosaic
	catapult.plugins.namespace
	catapult.plugins.transfer)
