add_target(
    catapult.chain LIBRARY
	LINK_LIBS 
		catapult.api
		catapult.cache_core
		catapult.cache_tx
		catapult.disruptor
		catapult.model
		catapult.observers
		catapult.utils
		catapult.validators	
    SOURCES ./
)
