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
#include "ContainerTypes.h"
#include "ResolverContext.h"
#include "TransactionRegistry.h"
#include "WeakEntityInfo.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/plugins.h"

namespace catapult {
	namespace model {
		struct EmbeddedTransaction;
		class NotificationSubscriber;
		struct Transaction;
	}
}

namespace catapult { namespace model {

	/// Transaction dependent attributes.
	struct TransactionAttributes {
		/// Minimum supported version.
		uint8_t MinVersion;

		/// Maximum supported version.
		uint8_t MaxVersion;

		/// Maximum transaction lifetime (optional).
		/// \note If \c 0, default network-specific maximum will be used.
		utils::TimeSpan MaxLifetime;
	};

	/// Contextual information passed to publish.
	struct PublishContext {
		/// Address of the published transaction signer.
		Address SignerAddress;
	};

	/// Typed transaction plugin.
	template<typename TTransaction>
	class PLUGIN_API_DEPENDENCY TransactionPluginT {
	public:
		virtual ~TransactionPluginT() = default;

	public:
		/// Gets the transaction entity type.
		virtual EntityType type() const = 0;

		/// Gets the transaction dependent attributes.
		virtual TransactionAttributes attributes() const = 0;

		/// Checks the real size of \a transaction against its reported size and returns \c true if the sizes match.
		virtual bool isSizeValid(const TTransaction& transaction) const = 0;
	};

	/// Embedded transaction plugin.
	class PLUGIN_API_DEPENDENCY EmbeddedTransactionPlugin : public TransactionPluginT<EmbeddedTransaction> {
	public:
		/// Sends all notifications from \a transaction with \a context to \a sub.
		virtual void publish(const EmbeddedTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) const = 0;

		/// Extracts addresses of additional accounts that must approve \a transaction.
		virtual UnresolvedAddressSet additionalRequiredCosignatories(const EmbeddedTransaction& transaction) const = 0;
	};

	/// Transaction plugin.
	class PLUGIN_API_DEPENDENCY TransactionPlugin : public TransactionPluginT<Transaction> {
	public:
		/// Sends all notifications from \a transactionInfo with \a context to \a sub.
		virtual void publish(
				const WeakEntityInfoT<Transaction>& transactionInfo,
				const PublishContext& context,
				NotificationSubscriber& sub) const = 0;

		/// Gets the number of embedded transactions in \a transaction.
		virtual uint32_t embeddedCount(const Transaction& transaction) const = 0;

		/// Extracts the primary data buffer from \a transaction that is used for signing and basic hashing.
		virtual RawBuffer dataBuffer(const Transaction& transaction) const = 0;

		/// Extracts additional buffers from \a transaction that should be included in the merkle hash in addition to
		/// the primary data buffer.
		virtual std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction& transaction) const = 0;

		/// \c true if this transaction type supports being embedded directly in blocks.
		virtual bool supportsTopLevel() const = 0;

		/// \c true if this transaction type supports being embedded in other transactions.
		virtual bool supportsEmbedding() const = 0;

		/// Gets the corresponding embedded plugin if supportsEmbedding() is \c true.
		virtual const EmbeddedTransactionPlugin& embeddedPlugin() const = 0;
	};

	/// Registry of transaction plugins.
	class TransactionRegistry : public TransactionRegistryT<TransactionPlugin> {};
}}
