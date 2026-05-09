set(TARGET_NAME catapult.mongo)

catapult_library_target(${TARGET_NAME} mappers storages)
# DO NOT alter the order, with model before plugins, linker is not pulling model::CreateNotificationPublisher producing invalid libextension.mongo as a result
# catapult.validators is needed due to validators::AggregateValidationResult
target_link_libraries(${TARGET_NAME} ${MongoDB_IMPORTED_TARGETS} catapult.extensions catapult.plugins catapult.model catapult.validators)
