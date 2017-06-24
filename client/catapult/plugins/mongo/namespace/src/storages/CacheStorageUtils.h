#pragma once
#include <vector>

namespace catapult {
	namespace state {
		struct MosaicDescriptor;
		class MosaicHistory;
		struct NamespaceDescriptor;
		class RootNamespaceHistory;
	}
}

namespace catapult { namespace mongo { namespace storages {

	/// Converts the mosaic \a history into a vector of mosaic descriptors.
	std::vector<state::MosaicDescriptor> MosaicDescriptorsFromHistory(const state::MosaicHistory& history);

	/// Converts the root namespace \a history into a vector of namespace descriptors.
	std::vector<state::NamespaceDescriptor> NamespaceDescriptorsFromHistory(const state::RootNamespaceHistory& history);
}}}
