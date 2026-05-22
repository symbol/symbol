add_target(
    catapult.mongo LIBRARY
	LINK_LIBS
		${MongoDB_IMPORTED_TARGETS}
		
		# DO NOT alter the order of the following! 
		# with model before plugins, linker is not pulling model::CreateNotificationPublisher 
		# producing invalid libextension.mongo as a result
		# catapult.validators is needed due to validators::AggregateValidationResult
		catapult.extensions 
		catapult.plugins 
		catapult.model 
		catapult.validators
	SOURCES 
		./
		./mappers
		./storages
)
