#pragma once
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/EntityInfo.h"

namespace catapult { namespace cache {

	/// Partial transactions change subscriber.
	class PtChangeSubscriber {
	public:
		using TransactionInfos = model::TransactionInfosSet;

	public:
		virtual ~PtChangeSubscriber() {}

	public:
		/// Indicates transaction infos (\a transactionInfos) were added to partial transactions.
		/// \note This is only aggregate part and will not have any cosignatures.
		virtual void notifyAddPartials(const TransactionInfos& transactionInfos) = 0;

		/// Indicates a cosignature (composed of \a signer and \a signature) was added to a partial transaction (\a parentTransactionInfo).
		virtual void notifyAddCosignature(
				const model::TransactionInfo& parentTransactionInfo,
				const Key& signer,
				const Signature& signature) = 0;

		/// Indicates transaction infos (\a transactionInfos) were removed from partial transactions.
		/// \note This is only aggregate part and will not have any cosignatures.
		virtual void notifyRemovePartials(const TransactionInfos& transactionInfos) = 0;

		/// Flushes all pending partial transaction changes.
		virtual void flush() = 0;
	};
}}
