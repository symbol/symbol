#include "AccountInfosSupplier.h"
#include "src/cache/AccountStateCache.h"

namespace catapult { namespace handlers {

	AccountInfosSupplier CreateAccountInfosSupplier(const cache::AccountStateCache& accountStateCache) {
		return [&accountStateCache](const auto& addresses) -> std::vector<std::shared_ptr<const model::AccountInfo>> {
			AccountInfosSupplier::result_type accountInfos;
			auto view = accountStateCache.createView();
			for (const auto& address : addresses) {
				const auto& pAccountState = view->findAccount(address);
				accountInfos.push_back(pAccountState ? pAccountState->toAccountInfo() : model::AccountInfo::FromAddress(address));
			}

			return accountInfos;
		};
	}
}}
