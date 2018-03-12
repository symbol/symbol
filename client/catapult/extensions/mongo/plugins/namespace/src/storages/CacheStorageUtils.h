#pragma once
#include "catapult/model/NetworkInfo.h"
#include "catapult/types.h"
#include <functional>
#include <vector>

namespace catapult {
	namespace mongo {
		namespace plugins {
			struct MosaicDescriptor;
			struct NamespaceDescriptor;
		}
	}
	namespace state {
		class MosaicHistory;
		class RootNamespaceHistory;
	}
}

namespace catapult { namespace mongo { namespace plugins {

	/// Converts the mosaic \a history into a vector of mosaic descriptors.
	std::vector<MosaicDescriptor> MosaicDescriptorsFromHistory(const state::MosaicHistory& history);

	/// Converts the root namespace \a history into a vector of namespace descriptors using \a networkIdentifier for address conversions.
	std::vector<NamespaceDescriptor> NamespaceDescriptorsFromHistory(
			const state::RootNamespaceHistory& history,
			model::NetworkIdentifier networkIdentifier);
}}}
