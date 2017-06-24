#include "MosaicInfosSupplier.h"
#include "src/cache/MosaicCache.h"
#include "src/model/MosaicInfo.h"
#include "catapult/utils/Casting.h"

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
	}

	MosaicInfosSupplier CreateMosaicInfosSupplier(const cache::MosaicCache& mosaicCache) {
		return [&mosaicCache](const auto& ids) {
			MosaicInfosSupplier::result_type mosaicInfos;
			auto view = mosaicCache.createView();
			for (auto id : ids) {
				if (!view->contains(id))
					mosaicInfos.push_back(MakeMosaicInfo(id, model::ArtifactInfoAttributes::None));
				else
					mosaicInfos.push_back(MakeMosaicInfo(view->get(id)));
			}

			return mosaicInfos;
		};
	}
}}
