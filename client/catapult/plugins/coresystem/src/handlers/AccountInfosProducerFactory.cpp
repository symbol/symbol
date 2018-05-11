/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "AccountInfosProducerFactory.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/handlers/BasicProducer.h"
#include "catapult/state/AccountStateAdapter.h"

namespace catapult { namespace handlers {

	namespace {
		class Producer : BasicProducer<model::AddressRange> {
		private:
			using ViewType = cache::LockedCacheView<cache::AccountStateCacheView>;

		public:
			Producer(ViewType&& view, const model::AddressRange& addresses)
					: BasicProducer<model::AddressRange>(addresses)
					, m_pView(std::make_shared<ViewType>(std::move(view)))
			{}

		public:
			auto operator()() {
				return next([&view = **m_pView](const auto& address) {
					const auto* pAccountState = view.tryGet(address);
					return pAccountState ? state::ToAccountInfo(*pAccountState) : model::AccountInfo::FromAddress(address);
				});
			}

		private:
			std::shared_ptr<ViewType> m_pView;
		};
	}

	AccountInfosProducerFactory CreateAccountInfosProducerFactory(const cache::AccountStateCache& accountStateCache) {
		return [&accountStateCache](const auto& addresses) {
			return Producer(accountStateCache.createView(), addresses);
		};
	}
}}
