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
#include "TransactionRegistry.h"
#include "WeakEntityInfo.h"
#include "catapult/types.h"

namespace catapult {
	namespace model {
		struct EmbeddedTransaction;
		class NotificationSubscriber;
		struct Transaction;
	}
}

namespace catapult { namespace model {

	/// A typed transaction plugin.
	template<typename TTransaction>
	class TransactionPluginT {
	public:
		virtual ~TransactionPluginT() {}

	public:
		/// Gets the transaction entity type.
		virtual EntityType type() const = 0;

		/// Calculates the real size of \a transaction.
		virtual uint64_t calculateRealSize(const TTransaction& transaction) const = 0;
	};

	/// An embedded transaction plugin.
	class EmbeddedTransactionPlugin : public TransactionPluginT<EmbeddedTransaction> {
	public:
		/// Sends all notifications from \a transaction to \a sub.
		virtual void publish(const EmbeddedTransaction& transaction, NotificationSubscriber& sub) const = 0;
	};

	/// A transaction plugin.
	class TransactionPlugin : public TransactionPluginT<Transaction> {
	public:
		/// Sends all notifications from \a transactionInfo to \a sub.
		virtual void publish(const WeakEntityInfoT<Transaction>& transactionInfo, NotificationSubscriber& sub) const = 0;

		/// Extracts the primary data buffer from \a transaction that is used for signing and basic hashing.
		virtual RawBuffer dataBuffer(const Transaction& transaction) const = 0;

		/// Extracts additional buffers from \a transaction that should be included in the merkle hash in addition to
		/// the primary data buffer.
		virtual std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction& transaction) const = 0;

		/// \c true if this transaction type supports embedding.
		virtual bool supportsEmbedding() const = 0;

		/// Gets the corresponding embedded plugin if supportsEmbedding() is \c true.
		virtual const EmbeddedTransactionPlugin& embeddedPlugin() const = 0;
	};

	/// A registry of transaction plugins.
	class TransactionRegistry : public TransactionRegistryT<TransactionPlugin> {
	};
}}
