#include "CacheStorageUtils.h"
#include "plugins/txes/namespace/src/state/MosaicDescriptor.h"
#include "plugins/txes/namespace/src/state/MosaicHistory.h"
#include "plugins/txes/namespace/src/state/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/RootNamespaceHistory.h"

namespace catapult { namespace mongo { namespace storages {

	std::vector<state::MosaicDescriptor> MosaicDescriptorsFromHistory(const state::MosaicHistory& history) {
		std::vector<state::MosaicDescriptor> descriptors;
		uint32_t index = 0;
		for (auto iter = history.cbegin(); history.cend() != iter; ++iter) {
			auto isActive = index == history.historyDepth() - 1;
			auto pEntry = std::shared_ptr<const state::MosaicEntry>(&(*iter), [](auto) {});
			descriptors.emplace_back(pEntry, index, isActive);
			++index;
		}

		return descriptors;
	}

	std::vector<state::NamespaceDescriptor> NamespaceDescriptorsFromHistory(const state::RootNamespaceHistory& history) {
		std::vector<state::NamespaceDescriptor> descriptors;
		uint32_t index = 0;
		state::Namespace::Path path;
		path.push_back(history.id());
		for (auto iter = history.cbegin(); history.cend() != iter; ++iter) {
			auto isActive = index == history.historyDepth() - 1;
			auto pRoot = std::shared_ptr<const state::RootNamespace>(&(*iter), [](auto) {});
			descriptors.emplace_back(path, pRoot, index, isActive);
			for (const auto& pair : iter->children())
				descriptors.emplace_back(pair.second, pRoot, index, isActive);

			++index;
		}

		return descriptors;
	}
}}}
