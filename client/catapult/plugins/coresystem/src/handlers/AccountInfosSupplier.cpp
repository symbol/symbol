#include "AccountInfosSupplier.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountStateAdapter.h"

namespace catapult { namespace handlers {

	AccountInfosSupplier CreateAccountInfosSupplier(const cache::AccountStateCache& accountStateCache) {
		return [&accountStateCache](const auto& addresses) {
			AccountInfosSupplier::result_type accountInfos;
			auto view = accountStateCache.createView();
			for (const auto& address : addresses) {
				const auto* pAccountState = view->tryGet(address);
				accountInfos.push_back(pAccountState ? state::ToAccountInfo(*pAccountState) : model::AccountInfo::FromAddress(address));
			}

			return accountInfos;
		};
	}
}}
