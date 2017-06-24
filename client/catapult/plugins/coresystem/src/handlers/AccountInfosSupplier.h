#pragma once
#include "catapult/model/RangeTypes.h"
#include <functional>
#include <vector>

namespace catapult {
	namespace cache { class AccountStateCache; }
	namespace model { struct AccountInfo; }
}

namespace catapult { namespace handlers {

	/// Prototype for a function that supplies a vector of account infos given a range of addresses.
	using AccountInfosSupplier = std::function<std::vector<std::shared_ptr<const model::AccountInfo>> (const model::AddressRange&)>;

	/// Creates a supplier that supplies account infos from \a accountStateCache.
	AccountInfosSupplier CreateAccountInfosSupplier(const cache::AccountStateCache& accountStateCache);
}}
