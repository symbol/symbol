#pragma once
#include "catapult/utils/Hashers.h"
#include "catapult/types.h"
#include <unordered_set>

namespace catapult {
	namespace model {
		template<typename TEntity>
		struct EntityInfoComparer;

		template<typename TEntity>
		struct EntityInfoHasher;
		struct Transaction;
		struct TransactionInfo;
	}
}

namespace catapult { namespace model {

	/// Unordered set of addresses.
	using AddressSet = std::unordered_set<Address, utils::ArrayHasher<Address>>;

	/// Unordered set of transaction infos.
	using TransactionInfosSet = std::unordered_set<
		TransactionInfo,
		EntityInfoHasher<const Transaction>,
		EntityInfoComparer<const Transaction>>;
}}

