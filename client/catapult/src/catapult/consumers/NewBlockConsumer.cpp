/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace consumers {

	namespace {
		using disruptor::InputSource;

		class NewBlockConsumer {
		public:
			NewBlockConsumer(const NewBlockSink& newBlockSink, InputSource sinkSourceMask)
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
				return CompleteSuccess();
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
