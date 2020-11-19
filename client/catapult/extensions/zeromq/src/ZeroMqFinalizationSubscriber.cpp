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

#include "ZeroMqFinalizationSubscriber.h"
#include "PackedFinalizedBlockHeader.h"
#include "ZeroMqEntityPublisher.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqFinalizationSubscriber : public subscribers::FinalizationSubscriber {
		public:
			explicit ZeroMqFinalizationSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyFinalizedBlock(const model::FinalizationRound& round, Height height, const Hash256& hash) override {
				m_publisher.publishFinalizedBlock({ round, height, hash });
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<subscribers::FinalizationSubscriber> CreateZeroMqFinalizationSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqFinalizationSubscriber>(publisher);
	}
}}
