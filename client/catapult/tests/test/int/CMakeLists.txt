cmake_minimum_required(VERSION 3.2)

catapult_library_target(tests.catapult.test.int)
target_link_libraries(tests.catapult.test.int
	tests.catapult.test.local
	catapult.plugins.transfer
	catapult.plugins.namespace) # shared lib dependency, allow tests to create transfer and namespace transaction plugins

include_directories(../../../plugins/txes/namespace)
