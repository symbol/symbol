#pragma once
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/EntityInfo.h"

namespace catapult { namespace cache {

	/// Unconfirmed transactions change subscriber.
	class UtChangeSubscriber {
	public:
		using TransactionInfos = model::TransactionInfosSet;

	public:
		virtual ~UtChangeSubscriber() {}

	public:
		/// Indicates transaction infos (\a transactionInfos) were added to unconfirmed transactions.
		virtual void notifyAdds(const TransactionInfos& transactionInfos) = 0;

		/// Indicates transaction infos (\a transactionInfos) were removed from unconfirmed transactions.
		virtual void notifyRemoves(const TransactionInfos& transactionInfos) = 0;

		/// Flushes all pending unconfirmed transaction changes.
		virtual void flush() = 0;
	};
}}
