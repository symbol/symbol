#include "NamespaceInfosSupplier.h"
#include "src/cache/NamespaceCache.h"
#include "src/model/NamespaceInfo.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace handlers {

	namespace {
		std::shared_ptr<const model::NamespaceInfo> MakeChildInfo(NamespaceId id, model::ArtifactInfoAttributes attributes) {
			auto pInfo = std::make_shared<model::NamespaceInfo>();
			pInfo->Size = sizeof(model::NamespaceInfo);
			pInfo->Id = id;
			pInfo->Attributes = attributes;
			pInfo->NumChildren = 0;
			return pInfo;
		}

		std::shared_ptr<const model::NamespaceInfo> MakeRootInfo(const state::RootNamespace& root) {
			auto numChildren = utils::checked_cast<size_t, uint16_t>(root.size());
			uint32_t entitySize = sizeof(model::NamespaceInfo) + numChildren * sizeof(NamespaceId);
			std::shared_ptr<model::NamespaceInfo> pInfo(reinterpret_cast<model::NamespaceInfo*>(::operator new(entitySize)));

			pInfo->Size = entitySize;
			pInfo->Id = root.id();
			pInfo->Attributes = model::ArtifactInfoAttributes::Is_Known;
			pInfo->NumChildren = numChildren;

			auto* pChildNamespaceId = pInfo->ChildrenPtr();
			for (const auto& pair : root.children())
				*pChildNamespaceId++ = pair.first;

			return pInfo;
		}
	}

	NamespaceInfosSupplier CreateNamespaceInfosSupplier(const cache::NamespaceCache& namespaceCache) {
		return [&namespaceCache](const auto& ids) {
			NamespaceInfosSupplier::result_type namespaceInfos;
			auto view = namespaceCache.createView();
			for (auto id : ids) {
				if (!view->contains(id)) {
					namespaceInfos.push_back(MakeChildInfo(id, model::ArtifactInfoAttributes::None));
					continue;
				}

				const auto& entry = view->get(id);
				if (!entry.ns().isRoot()) {
					namespaceInfos.push_back(MakeChildInfo(id, model::ArtifactInfoAttributes::Is_Known));
					continue;
				}

				namespaceInfos.push_back(MakeRootInfo(entry.root()));
			}

			return namespaceInfos;
		};
	}
}}
