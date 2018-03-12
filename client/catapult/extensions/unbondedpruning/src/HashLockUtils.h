#pragma once
#include "catapult/model/EntityInfo.h"
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace model { class NotificationPublisher; } }

namespace catapult { namespace unbondedpruning {

	/// Finds the hashes of all transactions dependent on \a transactionInfo using \a notificationPublisher.
	/// \note This function only finds dependent transactions by inspecting hash lock transactions.
	utils::HashSet FindDependentTransactionHashes(
			const model::TransactionInfo& transactionInfo,
			const model::NotificationPublisher& notificationPublisher);
}}
