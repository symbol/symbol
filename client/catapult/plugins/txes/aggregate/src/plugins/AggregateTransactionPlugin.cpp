/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "AggregateTransactionPlugin.h"
#include "src/model/AggregateNotifications.h"
#include "src/model/AggregateTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPlugin.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		constexpr const AggregateTransaction& CastToDerivedType(const Transaction& transaction) {
			return static_cast<const AggregateTransaction&>(transaction);
		}

		uint32_t CountTransactions(const AggregateTransaction& aggregate) {
			return static_cast<uint32_t>(std::distance(aggregate.Transactions().cbegin(), aggregate.Transactions().cend()));
		}

		class AggregateTransactionPlugin : public TransactionPlugin {
		public:
			AggregateTransactionPlugin(
					const TransactionRegistry& transactionRegistry,
					const utils::TimeSpan& maxTransactionLifetime,
					EntityType transactionType)
					: m_transactionRegistry(transactionRegistry)
					, m_maxTransactionLifetime(maxTransactionLifetime)
					, m_transactionType(transactionType)
			{}

		public:
			EntityType type() const override {
				return m_transactionType;
			}

			TransactionAttributes attributes() const override {
				auto version = AggregateTransaction::Current_Version;
				return { version, version, m_maxTransactionLifetime };
			}

			bool isSizeValid(const Transaction& transaction) const override {
				return IsSizeValid(CastToDerivedType(transaction), m_transactionRegistry);
			}

			void publish(
					const WeakEntityInfoT<Transaction>& transactionInfo,
					const PublishContext&,
					NotificationSubscriber& sub) const override {
				const auto& aggregate = CastToDerivedType(transactionInfo.entity());

				// publish aggregate notifications
				// (notice that this must be raised before embedded transaction notifications in order for cosignatory aggregation to work)
				auto numCosignatures = aggregate.CosignaturesCount();
				auto numTransactions = CountTransactions(aggregate);
				sub.notify(AggregateCosignaturesNotification(
						aggregate.SignerPublicKey,
						numTransactions,
						aggregate.TransactionsPtr(),
						numCosignatures,
						aggregate.CosignaturesPtr()));

				sub.notify(AggregateEmbeddedTransactionsNotification(
						aggregate.TransactionsHash,
						numTransactions,
						aggregate.TransactionsPtr()));

				// publish all sub-transaction information
				for (const auto& subTransaction : aggregate.Transactions()) {
					// - change source
					constexpr auto Relative = SourceChangeNotification::SourceChangeType::Relative;
					sub.notify(SourceChangeNotification(Relative, 0, Relative, 1));

					// - signers and entity
					PublishNotifications(subTransaction, sub);
					const auto& plugin = m_transactionRegistry.findPlugin(subTransaction.Type)->embeddedPlugin();
					auto subTransactionAttributes = plugin.attributes();

					sub.notify(EntityNotification(
							subTransaction.Network,
							subTransaction.Type,
							subTransaction.Version,
							subTransactionAttributes.MinVersion,
							subTransactionAttributes.MaxVersion));

					// - generic sub-transaction notification
					sub.notify(AggregateEmbeddedTransactionNotification(
							aggregate.SignerPublicKey,
							subTransaction,
							numCosignatures,
							aggregate.CosignaturesPtr()));

					// - specific sub-transaction notifications
					//   (calculateRealSize would have failed if plugin is unknown or not embeddable)
					PublishContext subContext;
					subContext.SignerAddress = GetSignerAddress(subTransaction);
					plugin.publish(subTransaction, subContext, sub);
				}

				// publish all cosignatory information (as an optimization these are published with the source of the last sub-transaction)
				const auto* pCosignature = aggregate.CosignaturesPtr();
				for (auto i = 0u; i < numCosignatures; ++i) {
					// - notice that all valid cosignatories must have been observed previously as part of either
					//   (1) sub-transaction execution or (2) composite account setup
					// - require the cosignatories to sign the aggregate indirectly via the hash of its data
					sub.notify(InternalPaddingNotification(pCosignature->Version));
					sub.notify(SignatureNotification(pCosignature->SignerPublicKey, pCosignature->Signature, transactionInfo.hash()));
					++pCosignature;
				}
			}

			uint32_t embeddedCount(const Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);
				const auto& transactions = aggregate.Transactions();
				return static_cast<uint32_t>(std::distance(transactions.cbegin(), transactions.cend()));
			}

			RawBuffer dataBuffer(const Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);

				auto headerSize = VerifiableEntity::Header_Size;
				return {
					reinterpret_cast<const uint8_t*>(&aggregate) + headerSize,
					sizeof(AggregateTransaction) - headerSize - AggregateTransaction::Footer_Size
				};
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);

				std::vector<RawBuffer> buffers;
				auto numCosignatures = aggregate.CosignaturesCount();
				const auto* pCosignature = aggregate.CosignaturesPtr();
				for (auto i = 0u; i < numCosignatures; ++i) {
					buffers.push_back(pCosignature->SignerPublicKey);
					++pCosignature;
				}

				return buffers;
			}

			bool supportsTopLevel() const override {
				return true;
			}

			bool supportsEmbedding() const override {
				return false;
			}

			const EmbeddedTransactionPlugin& embeddedPlugin() const override {
				CATAPULT_THROW_RUNTIME_ERROR("aggregate transaction is not embeddable");
			}

		private:
			const TransactionRegistry& m_transactionRegistry;
			utils::TimeSpan m_maxTransactionLifetime;
			EntityType m_transactionType;
		};
	}

	std::unique_ptr<TransactionPlugin> CreateAggregateTransactionPlugin(
			const TransactionRegistry& transactionRegistry,
			EntityType transactionType) {
		return CreateAggregateTransactionPlugin(transactionRegistry, utils::TimeSpan(), transactionType);
	}

	std::unique_ptr<TransactionPlugin> CreateAggregateTransactionPlugin(
			const TransactionRegistry& transactionRegistry,
			const utils::TimeSpan& maxTransactionLifetime,
			EntityType transactionType) {
		return std::make_unique<AggregateTransactionPlugin>(transactionRegistry, maxTransactionLifetime, transactionType);
	}
}}
