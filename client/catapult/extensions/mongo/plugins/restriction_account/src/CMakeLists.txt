set(PLUGIN_BASE_NAME catapult.mongo.plugins.restrictionaccount)

catapult_mongo_plugin_src_with_deps(${PLUGIN_BASE_NAME} mappers storages)

# restrictionaccount.deps is needed because the mongo account restriction cache storage needs the account restriction cache
target_link_libraries(${PLUGIN_BASE_NAME}.deps catapult.plugins.restrictionaccount.deps)
