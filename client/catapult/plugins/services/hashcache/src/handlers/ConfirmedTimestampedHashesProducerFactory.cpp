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

#include "ConfirmedTimestampedHashesProducerFactory.h"
#include "src/cache/HashCache.h"
#include "catapult/handlers/BasicProducer.h"

namespace catapult { namespace handlers {

	namespace {
		class Producer : BasicProducer<state::TimestampedHashRange> {
		private:
			using ViewType = cache::LockedCacheView<cache::HashCacheView>;

		public:
			Producer(ViewType&& view, const state::TimestampedHashRange& timestampedHashes)
					: BasicProducer<state::TimestampedHashRange>(timestampedHashes)
					, m_pView(std::make_shared<ViewType>(std::move(view)))
			{}

		public:
			auto operator()() {
				auto isUnknown = false;
				const state::TimestampedHash* pNextTimestampedHash = nullptr;
				while (!isUnknown) {
					pNextTimestampedHash = next([&view = **m_pView, &isUnknown](const auto& timestampedHash) {
						if (!view.contains(timestampedHash))
							isUnknown = true;

						return &timestampedHash;
					});

					// if nullptr, the producer is depleted
					if (!pNextTimestampedHash)
						break;
				}

				return pNextTimestampedHash;
			}

		private:
			std::shared_ptr<ViewType> m_pView;
		};
	}

	ConfirmedTimestampedHashesProducerFactory CreateConfirmedTimestampedHashesProducerFactory(const cache::HashCache& hashCache) {
		return [&hashCache](const auto& timestampedHashes) {
			return Producer(hashCache.createView(), timestampedHashes);
		};
	}
}}
