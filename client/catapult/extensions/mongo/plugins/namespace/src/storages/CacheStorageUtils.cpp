#include "CacheStorageUtils.h"
#include "src/mappers/MosaicDescriptor.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/MosaicHistory.h"
#include "plugins/txes/namespace/src/state/RootNamespaceHistory.h"
#include "catapult/model/Address.h"

namespace catapult { namespace mongo { namespace plugins {

	std::vector<MosaicDescriptor> MosaicDescriptorsFromHistory(const state::MosaicHistory& history) {
		std::vector<MosaicDescriptor> descriptors;
		uint32_t index = 0;
		for (const auto& mosaicEntry : history) {
			auto isActive = index == history.historyDepth() - 1;
			auto pEntry = std::shared_ptr<const state::MosaicEntry>(&mosaicEntry, [](auto) {});
			descriptors.emplace_back(pEntry, index, isActive);
			++index;
		}

		return descriptors;
	}

	std::vector<NamespaceDescriptor> NamespaceDescriptorsFromHistory(
			const state::RootNamespaceHistory& history,
			model::NetworkIdentifier networkIdentifier) {
		std::vector<NamespaceDescriptor> descriptors;
		uint32_t index = 0;
		state::Namespace::Path path;
		path.push_back(history.id());
		for (const auto& rootNamespaceEntry : history) {
			auto isActive = index == history.historyDepth() - 1;
			auto address = model::PublicKeyToAddress(rootNamespaceEntry.owner(), networkIdentifier);
			auto pRoot = std::shared_ptr<const state::RootNamespace>(&rootNamespaceEntry, [](auto) {});
			descriptors.emplace_back(path, pRoot, address, index, isActive);
			for (const auto& pair : rootNamespaceEntry.children())
				descriptors.emplace_back(pair.second, pRoot, address, index, isActive);

			++index;
		}

		return descriptors;
	}
}}}
