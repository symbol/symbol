#pragma once
#include "ContiguousEntityContainer.h"
#include "SizePrefixedEntity.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a transaction container.
	/// \note This container only assumes components of type 'SizePrefixedEntity' but is named 'Transaction' because all current
	///       usages are containers of some type of transactions and this naming leads to more natural usage
	///       (i.e. Transactions vs Entities).
	template<typename TEntityHeader, typename TComponentEntity>
	struct TransactionContainer : public TEntityHeader {
		/// Returns a container wrapping the transactions contained in this container with the
		/// desired error policy (\a errorPolicy).
		auto Transactions(EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) {
			return MakeContiguousEntityContainer(TransactionsPtr(), GetTransactionPayloadSize(*this), errorPolicy);
		}

		/// Returns a container wrapping the const transactions contained in this container with the
		/// desired error policy (\a errorPolicy).
		auto Transactions(EntityContainerErrorPolicy errorPolicy = EntityContainerErrorPolicy::Throw) const {
			return MakeContiguousEntityContainer(TransactionsPtr(), GetTransactionPayloadSize(*this), errorPolicy);
		}

		/// Returns a pointer to transactions contained in this container.
		TComponentEntity* TransactionsPtr() {
			return TEntityHeader::Size <= sizeof(TEntityHeader)
					? nullptr
					: reinterpret_cast<TComponentEntity*>(reinterpret_cast<uint8_t*>(this) + sizeof(TEntityHeader));
		}

		/// Returns a const pointer to transactions contained in this container.
		const TComponentEntity* TransactionsPtr() const {
			return TEntityHeader::Size <= sizeof(TEntityHeader)
					? nullptr
					: reinterpret_cast<const TComponentEntity*>(reinterpret_cast<const uint8_t*>(this) + sizeof(TEntityHeader));
		}

		// followed by "transactions"
	};

#pragma pack(pop)
}}
