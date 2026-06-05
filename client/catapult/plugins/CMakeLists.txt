function(derive_plugin_full_name OUT_VAR)
	cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME _plugin_name)
	string(REPLACE "_" "" _plugin_name ${_plugin_name})
	set(${OUT_VAR} ${_plugin_base_name}.${_plugin_name} PARENT_SCOPE)
endfunction()

set(PLUGIN_CATAPULT_LIBS
	${CORE_CATAPULT_LIBS}
	catapult.cache
	catapult.cache_core
	catapult.config
	catapult.observers
	catapult.plugins
	catapult.state
	catapult.validators)

set(_plugin_base_name catapult.plugins)
add_subdirs()
