/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/model/Cosignature.h"
#include "catapult/model/EmbeddedTransaction.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region aggregate notification types

/// Defines an aggregate notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_AGGREGATE_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Aggregate, DESCRIPTION, CODE)

	/// Aggregate was received with cosignatures.
	DEFINE_AGGREGATE_NOTIFICATION(Cosignatures, 0x0001, Validator);

	/// Aggregate was received with an embedded transaction.
	DEFINE_AGGREGATE_NOTIFICATION(EmbeddedTransaction, 0x0002, Validator);

#undef DEFINE_AGGREGATE_NOTIFICATION

	// endregion

	// region BasicAggregateNotification

	/// A basic aggregate notification.
	template<typename TDerivedNotification>
	struct BasicAggregateNotification : public Notification {
	protected:
		/// Creates a notification around \a signer, \a cosignaturesCount and \a pCosignatures.
		BasicAggregateNotification(const Key& signer, size_t cosignaturesCount, const Cosignature* pCosignatures)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Signer(signer)
				, CosignaturesCount(cosignaturesCount)
				, CosignaturesPtr(pCosignatures)
		{}

	public:
		/// Aggregate signer.
		const Key& Signer;

		/// Number of cosignatures.
		size_t CosignaturesCount;

		/// Const pointer to the first cosignature.
		const Cosignature* CosignaturesPtr;
	};

	// endregion

	// region AggregateEmbeddedTransactionNotification

	/// Notification of an embedded aggregate transaction with cosignatures.
	struct AggregateEmbeddedTransactionNotification : public BasicAggregateNotification<AggregateEmbeddedTransactionNotification> {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Aggregate_EmbeddedTransaction_Notification;

	public:
		/// Creates a notification around \a signer, \a transaction, \a cosignaturesCount and \a pCosignatures.
		AggregateEmbeddedTransactionNotification(
				const Key& signer,
				const EmbeddedTransaction& transaction,
				size_t cosignaturesCount,
				const Cosignature* pCosignatures)
				: BasicAggregateNotification<AggregateEmbeddedTransactionNotification>(signer, cosignaturesCount, pCosignatures)
				, Transaction(transaction)
		{}

	public:
		/// Embedded transaction.
		const EmbeddedTransaction& Transaction;
	};

	// endregion

	// region AggregateCosignaturesNotification

	/// Notification of an aggregate transaction with transactions and cosignatures.
	/// \note TransactionsPtr and CosignaturesPtr are provided instead of minimally required keys in order to support undoing.
	struct AggregateCosignaturesNotification : public BasicAggregateNotification<AggregateCosignaturesNotification> {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Aggregate_Cosignatures_Notification;

	public:
		/// Creates a notification around \a signer, \a transactionsCount, \a pTransactions, \a cosignaturesCount and \a pCosignatures.
		AggregateCosignaturesNotification(
				const Key& signer,
				size_t transactionsCount,
				const EmbeddedTransaction* pTransactions,
				size_t cosignaturesCount,
				const Cosignature* pCosignatures)
				: BasicAggregateNotification<AggregateCosignaturesNotification>(signer, cosignaturesCount, pCosignatures)
				, TransactionsCount(transactionsCount)
				, TransactionsPtr(pTransactions)
		{}

	public:
		/// Number of transactions.
		size_t TransactionsCount;

		/// Const pointer to the first transaction.
		const EmbeddedTransaction* TransactionsPtr;
	};

	// endregion
}}
