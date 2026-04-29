set(PLUGIN_SDK_FOLDERS model state)
set(PLUGIN_DEPS_FOLDERS cache config observers validators)

include_directories(.)
subdirectories(ADD BASE . VERBOSE)
