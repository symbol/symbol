#pragma once
#include "src/model/NamespaceConstants.h"
#include "catapult/model/EntityRange.h"
#include "catapult/types.h"
#include <functional>
#include <vector>

namespace catapult {
	namespace cache { class NamespaceCache; }
	namespace model { struct NamespaceInfo; }
}

namespace catapult { namespace handlers {

	/// Prototype for a function that supplies a vector of namespace infos given a range of ids.
	using NamespaceInfosSupplier =
			std::function<std::vector<std::shared_ptr<const model::NamespaceInfo>> (const model::EntityRange<NamespaceId>&)>;

	/// Creates a supplier that supplies namespace infos from \a namespaceCache.
	NamespaceInfosSupplier CreateNamespaceInfosSupplier(const cache::NamespaceCache& namespaceCache);
}}
