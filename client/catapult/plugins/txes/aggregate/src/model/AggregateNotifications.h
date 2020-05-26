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
	DEFINE_AGGREGATE_NOTIFICATION(Embedded_Transaction, 0x0002, Validator);

	/// Aggregate was received with embedded transactions.
	DEFINE_AGGREGATE_NOTIFICATION(Embedded_Transactions, 0x0003, Validator);

#undef DEFINE_AGGREGATE_NOTIFICATION

	// endregion

	// region BasicAggregateNotification

	/// Basic aggregate notification.
	template<typename TDerivedNotification>
	struct BasicAggregateNotification : public Notification {
	protected:
		/// Creates a notification around \a signerPublicKey, \a cosignaturesCount and \a pCosignatures.
		BasicAggregateNotification(const Key& signerPublicKey, size_t cosignaturesCount, const Cosignature* pCosignatures)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, SignerPublicKey(signerPublicKey)
				, CosignaturesCount(cosignaturesCount)
				, CosignaturesPtr(pCosignatures)
		{}

	public:
		/// Aggregate signer public key.
		const Key& SignerPublicKey;

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
		static constexpr auto Notification_Type = Aggregate_Embedded_Transaction_Notification;

	public:
		/// Creates a notification around \a signerPublicKey, \a transaction, \a cosignaturesCount and \a pCosignatures.
		AggregateEmbeddedTransactionNotification(
				const Key& signerPublicKey,
				const EmbeddedTransaction& transaction,
				size_t cosignaturesCount,
				const Cosignature* pCosignatures)
				: BasicAggregateNotification<AggregateEmbeddedTransactionNotification>(signerPublicKey, cosignaturesCount, pCosignatures)
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
		/// Creates a notification around \a signerPublicKey, \a transactionsCount, \a pTransactions, \a cosignaturesCount
		/// and \a pCosignatures.
		AggregateCosignaturesNotification(
				const Key& signerPublicKey,
				size_t transactionsCount,
				const EmbeddedTransaction* pTransactions,
				size_t cosignaturesCount,
				const Cosignature* pCosignatures)
				: BasicAggregateNotification<AggregateCosignaturesNotification>(signerPublicKey, cosignaturesCount, pCosignatures)
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

	// region AggregateEmbeddedTransactionsNotification

	/// Notification of an aggregate transaction with transactions.
	struct AggregateEmbeddedTransactionsNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Aggregate_Embedded_Transactions_Notification;

	public:
		/// Creates a notification around \a transactionsHash, \a transactionsCount and \a pTransactions.
		AggregateEmbeddedTransactionsNotification(
				const Hash256& transactionsHash,
				size_t transactionsCount,
				const EmbeddedTransaction* pTransactions)
				: Notification(Notification_Type, sizeof(AggregateEmbeddedTransactionsNotification))
				, TransactionsHash(transactionsHash)
				, TransactionsCount(transactionsCount)
				, TransactionsPtr(pTransactions)
		{}

	public:
		/// Aggregate transactions hash.
		const Hash256& TransactionsHash;

		/// Number of transactions.
		size_t TransactionsCount;

		/// Const pointer to the first transaction.
		const EmbeddedTransaction* TransactionsPtr;
	};

	// endregion
}}
