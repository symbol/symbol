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

#include "MosaicInfosProducerFactory.h"
#include "src/cache/MosaicCache.h"
#include "src/model/MosaicInfo.h"
#include "catapult/handlers/BasicProducer.h"

namespace catapult { namespace handlers {

	namespace {
		std::shared_ptr<model::MosaicInfo> MakeMosaicInfo(MosaicId id, model::ArtifactInfoAttributes attributes) {
			auto pInfo = std::make_shared<model::MosaicInfo>();
			pInfo->Attributes = attributes;
			pInfo->Id = id;
			return pInfo;
		}

		std::shared_ptr<model::MosaicInfo> MakeMosaicInfo(const state::MosaicEntry& entry) {
			auto pInfo = std::make_shared<model::MosaicInfo>();
			pInfo->Attributes = model::ArtifactInfoAttributes::Is_Known;

			pInfo->NamespaceId = entry.namespaceId();
			pInfo->Id = entry.mosaicId();
			pInfo->Supply = entry.supply();
			return pInfo;
		}

		class Producer : BasicProducer<model::EntityRange<MosaicId>> {
		private:
			using ViewType = cache::LockedCacheView<cache::MosaicCacheView>;

		public:
			Producer(ViewType&& view, const model::EntityRange<MosaicId>& ids)
					: BasicProducer<model::EntityRange<MosaicId>>(ids)
					, m_pView(std::make_shared<ViewType>(std::move(view)))
			{}

		public:
			auto operator()() {
				return next([&view = **m_pView](auto id) {
					return !view.contains(id)
							? MakeMosaicInfo(id, model::ArtifactInfoAttributes::None)
							: MakeMosaicInfo(view.get(id));
				});
			}

		private:
			std::shared_ptr<ViewType> m_pView;
		};
	}

	MosaicInfosProducerFactory CreateMosaicInfosProducerFactory(const cache::MosaicCache& mosaicCache) {
		return [&mosaicCache](const auto& ids) {
			return Producer(mosaicCache.createView(), ids);
		};
	}
}}
