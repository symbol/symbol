#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace consumers {

	namespace {
		using disruptor::InputSource;

		class NewBlockConsumer {
		public:
			explicit NewBlockConsumer(const NewBlockSink& newBlockSink, InputSource sinkSourceMask)
					: m_newBlockSink(newBlockSink)
					, m_sinkSourceMask(sinkSourceMask)
			{}

		public:
			ConsumerResult operator()(disruptor::ConsumerInput& input) const {
				if (input.empty())
					return Abort(Failure_Consumer_Empty_Input);

				// only (single) blocks with a configured source are forwarded
				if (1 != input.blocks().size() || !isConfigured(input.source()))
					return Continue();

				// 1. split up the input into its component blocks
				//    - detachBlockRange transfers ownership of the range from the input
				//      but doesn't invalidate the input elements
				//    - the range is moved into ExtractEntitiesFromRange, which extends the lifetime of the range
				//      to the lifetime of the returned blocks
				auto blocks = model::BlockRange::ExtractEntitiesFromRange(input.detachBlockRange());
				auto pNewBlock = blocks.front();
				CATAPULT_LOG(debug) << "forwarding a new block with height " << pNewBlock->Height;
				m_newBlockSink(pNewBlock);

				// 2. indicate input was consumed and processing is complete
				return Complete();
			}

		private:
			bool isConfigured(InputSource source) const {
				return 0 != (utils::to_underlying_type(m_sinkSourceMask) & utils::to_underlying_type(source));
			}

		private:
			NewBlockSink m_newBlockSink;
			InputSource m_sinkSourceMask;
		};
	}

	disruptor::DisruptorConsumer CreateNewBlockConsumer(const NewBlockSink& newBlockSink, InputSource sinkSourceMask) {
		return NewBlockConsumer(newBlockSink, sinkSourceMask);
	}
}}
