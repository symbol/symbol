#pragma once
#include "Cosignature.h"
#include "catapult/model/EmbeddedEntity.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region aggregate notification types

/// Defines an aggregate notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_AGGREGATE_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Aggregate, DESCRIPTION, CODE)

	/// Aggregate was received with cosignatures.
	DEFINE_AGGREGATE_NOTIFICATION(Cosignatures, 0x001, Validator);

	/// Aggregate was received with an embedded transaction.
	DEFINE_AGGREGATE_NOTIFICATION(EmbeddedTransaction, 0x002, Validator);

#undef DEFINE_AGGREGATE_NOTIFICATION

	// endregion

	/// A basic aggregate notification.
	template<typename TDerivedNotification>
	struct BasicAggregateNotification : public Notification {
	public:
		/// Creates a notification around \a signer, \a cosignaturesCount and \a pCosignatures.
		explicit BasicAggregateNotification(const Key& signer, size_t cosignaturesCount, const Cosignature* pCosignatures)
				: Notification(TDerivedNotification::Notification_Type, sizeof(TDerivedNotification))
				, Signer(signer)
				, CosignaturesCount(cosignaturesCount)
				, CosignaturesPtr(pCosignatures)
		{}

	public:
		/// The aggregate signer.
		const Key& Signer;

		/// The number of cosignatures.
		size_t CosignaturesCount;

		/// Const pointer to the first cosignature.
		const Cosignature* CosignaturesPtr;
	};

	/// Notification of an embedded aggregate transaction with cosignatures.
	struct AggregateEmbeddedTransactionNotification : public BasicAggregateNotification<AggregateEmbeddedTransactionNotification> {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Aggregate_EmbeddedTransaction_Notification;

	public:
		/// Creates a notification around \a signer, \a transaction, \a cosignaturesCount and \a pCosignatures.
		explicit AggregateEmbeddedTransactionNotification(
				const Key& signer,
				const EmbeddedEntity& transaction,
				size_t cosignaturesCount,
				const Cosignature* pCosignatures)
				: BasicAggregateNotification<AggregateEmbeddedTransactionNotification>(signer, cosignaturesCount, pCosignatures)
				, Transaction(transaction)
		{}

	public:
		/// The embedded transaction.
		const EmbeddedEntity& Transaction;
	};

	/// Notification of an aggregate transaction with transactions and cosignatures.
	/// \note TransactionsPtr and CosignaturesPtr are provided instead of minimally required keys in order to support undoing.
	struct AggregateCosignaturesNotification : public BasicAggregateNotification<AggregateCosignaturesNotification> {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Aggregate_Cosignatures_Notification;

	public:
		/// Creates a notification around \a signer, \a transactionsCount, \a pTransactions, \a cosignaturesCount and \a pCosignatures.
		explicit AggregateCosignaturesNotification(
				const Key& signer,
				size_t transactionsCount,
				const EmbeddedEntity* pTransactions,
				size_t cosignaturesCount,
				const Cosignature* pCosignatures)
				: BasicAggregateNotification<AggregateCosignaturesNotification>(signer, cosignaturesCount, pCosignatures)
				, TransactionsCount(transactionsCount)
				, TransactionsPtr(pTransactions)
		{}

	public:
		/// The number of transactions.
		size_t TransactionsCount;

		/// Const pointer to the first transaction.
		const EmbeddedEntity* TransactionsPtr;
	};
}}
