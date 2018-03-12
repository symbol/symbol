cmake_minimum_required(VERSION 3.2)

set(PLUGIN_CATAPULT_LIBS catapult.mongo)

add_subdirectory(aggregate)
add_subdirectory(lock)
add_subdirectory(multisig)
add_subdirectory(namespace)
add_subdirectory(transfer)
