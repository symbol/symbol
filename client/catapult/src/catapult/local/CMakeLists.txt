cmake_minimum_required(VERSION 3.2)

catapult_library_target(catapult.local)
target_link_libraries(catapult.local
	catapult.config
	catapult.consumers
	catapult.extensions
	catapult.handlers
	catapult.net
	catapult.plugins)
