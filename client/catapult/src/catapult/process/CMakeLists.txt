catapult_library_target(catapult.process)
target_link_libraries(catapult.process catapult.version)

subdirectories(ADD)

install(TARGETS catapult.broker catapult.importer catapult.recovery catapult.server)
