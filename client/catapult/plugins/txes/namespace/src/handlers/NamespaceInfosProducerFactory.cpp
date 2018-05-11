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

#include "NamespaceInfosProducerFactory.h"
#include "src/cache/NamespaceCache.h"
#include "src/model/NamespaceInfo.h"
#include "catapult/handlers/BasicProducer.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace handlers {

	namespace {
		std::shared_ptr<const model::NamespaceInfo> MakeChildInfo(NamespaceId id, model::ArtifactInfoAttributes attributes) {
			auto pInfo = std::make_shared<model::NamespaceInfo>();
			pInfo->Size = sizeof(model::NamespaceInfo);
			pInfo->Id = id;
			pInfo->Attributes = attributes;
			pInfo->ChildCount = 0;
			return pInfo;
		}

		std::shared_ptr<const model::NamespaceInfo> MakeRootInfo(const state::RootNamespace& root) {
			auto numChildren = utils::checked_cast<size_t, uint16_t>(root.size());
			uint32_t entitySize = sizeof(model::NamespaceInfo) + numChildren * sizeof(NamespaceId);
			auto pInfo = utils::MakeSharedWithSize<model::NamespaceInfo>(entitySize);

			pInfo->Size = entitySize;
			pInfo->Id = root.id();
			pInfo->Attributes = model::ArtifactInfoAttributes::Is_Known;
			pInfo->ChildCount = numChildren;

			auto* pChildNamespaceId = pInfo->ChildrenPtr();
			for (const auto& pair : root.children())
				*pChildNamespaceId++ = pair.first;

			return pInfo;
		}

		class Producer : BasicProducer<model::EntityRange<NamespaceId>> {
		private:
			using ViewType = cache::LockedCacheView<cache::NamespaceCacheView>;

		public:
			Producer(ViewType&& view, const model::EntityRange<NamespaceId>& ids)
					: BasicProducer<model::EntityRange<NamespaceId>>(ids)
					, m_pView(std::make_shared<ViewType>(std::move(view)))
			{}

		public:
			auto operator()() {
				return next([&view = **m_pView](auto id) {
					if (!view.contains(id))
						return MakeChildInfo(id, model::ArtifactInfoAttributes::None);

					const auto& entry = view.get(id);
					if (!entry.ns().isRoot())
						return MakeChildInfo(id, model::ArtifactInfoAttributes::Is_Known);

					return MakeRootInfo(entry.root());
				});
			}

		private:
			std::shared_ptr<ViewType> m_pView;
		};
	}

	NamespaceInfosProducerFactory CreateNamespaceInfosProducerFactory(const cache::NamespaceCache& namespaceCache) {
		return [&namespaceCache](const auto& ids) {
			return Producer(namespaceCache.createView(), ids);
		};
	}
}}
