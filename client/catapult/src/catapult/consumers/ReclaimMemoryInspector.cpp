#include "ReclaimMemoryInspector.h"
#include "catapult/validators/ValidationResult.h"

namespace catapult { namespace consumers {

	namespace {
		class ReclaimMemoryInspector {
		public:
			void operator()(disruptor::ConsumerInput& input, const disruptor::ConsumerCompletionResult& completionResult) const {
				if (disruptor::CompletionStatus::Aborted == completionResult.CompletionStatus) {
					auto validationResult = static_cast<validators::ValidationResult>(completionResult.CompletionCode);
					CATAPULT_LOG_LEVEL(MapToLogLevel(validationResult))
							<< "consumer aborted at position " << completionResult.FinalConsumerPosition
							<< " while processing " << input
							<< " due to " << validationResult;
				}

				input = disruptor::ConsumerInput();
			}
		};
	}

	disruptor::DisruptorInspector CreateReclaimMemoryInspector() {
		return ReclaimMemoryInspector();
	}
}}
