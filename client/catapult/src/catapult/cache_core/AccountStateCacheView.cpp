#include "AccountStateCacheView.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace cache {

	BasicAccountStateCacheView::BasicAccountStateCacheView(
			const AccountStateCacheTypes::BaseSetType& accountStateSets,
			const AccountStateCacheTypes::Options& options,
			const model::AddressSet& highValueAddresses)
			: BasicAccountStateCacheView(
					accountStateSets,
					options,
					highValueAddresses,
					std::make_unique<AccountStateCacheViewMixins::KeyLookupAdapter>(
							accountStateSets.KeyLookupMap,
							accountStateSets.Primary))
	{}

	BasicAccountStateCacheView::BasicAccountStateCacheView(
			const AccountStateCacheTypes::BaseSetType& accountStateSets,
			const AccountStateCacheTypes::Options& options,
			const model::AddressSet&,
			std::unique_ptr<AccountStateCacheViewMixins::KeyLookupAdapter>&& pKeyLookupAdapter)
			: AccountStateCacheViewMixins::Size(accountStateSets.Primary)
			, AccountStateCacheViewMixins::ContainsAddress(accountStateSets.Primary)
			, AccountStateCacheViewMixins::ContainsKey(accountStateSets.KeyLookupMap)
			, AccountStateCacheViewMixins::MapIteration(accountStateSets.Primary)
			, AccountStateCacheViewMixins::ConstAccessorAddress(accountStateSets.Primary)
			, AccountStateCacheViewMixins::ConstAccessorKey(*pKeyLookupAdapter)
			, m_networkIdentifier(options.NetworkIdentifier)
			, m_importanceGrouping(options.ImportanceGrouping)
			, m_pKeyLookupAdapter(std::move(pKeyLookupAdapter))
	{}

	model::NetworkIdentifier BasicAccountStateCacheView::networkIdentifier() const {
		return m_networkIdentifier;
	}

	uint64_t BasicAccountStateCacheView::importanceGrouping() const {
		return m_importanceGrouping;
	}
}}
