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

#include "VotingKeyLinkTransactionPlugin.h"
#include "src/model/KeyLinkNotifications.h"
#include "src/model/VotingKeyLinkTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, const PublishContext&, NotificationSubscriber& sub) {
			sub.notify(KeyLinkActionNotification(transaction.LinkAction));
			sub.notify(VotingKeyLinkNotification(
					transaction.SignerPublicKey,
					{ transaction.LinkedPublicKey, transaction.StartEpoch, transaction.EndEpoch },
					transaction.LinkAction));
		}

		class AggregateEmbededTransactionPlugin : public EmbeddedTransactionPlugin {
		public:
			AggregateEmbededTransactionPlugin(
					const EmbeddedTransactionPlugin& transactionPluginV1,
					const EmbeddedTransactionPlugin& transactionPluginV2)
					: m_transactionPluginV1(transactionPluginV1)
					, m_transactionPluginV2(transactionPluginV2)
			{}

		private:
			const auto& transactionPlugin(const EmbeddedTransaction& transaction) const {
				return 1 == transaction.Version ? m_transactionPluginV1 : m_transactionPluginV2;
			}

		public:
			EntityType type() const override {
				return m_transactionPluginV2.type();
			}

			TransactionAttributes attributes() const override {
				return { 1, 2, m_transactionPluginV2.attributes().MaxLifetime };
			}

			bool isSizeValid(const EmbeddedTransaction& transaction) const override {
				return sizeof(EmbeddedTransaction) <= transaction.Size && transactionPlugin(transaction).isSizeValid(transaction);
			}

		public:
			void publish(
					const EmbeddedTransaction& transaction,
					const PublishContext& context,
					NotificationSubscriber& sub) const override {
				transactionPlugin(transaction).publish(transaction, context, sub);
			}

			UnresolvedAddressSet additionalRequiredCosignatories(const EmbeddedTransaction& transaction) const override {
				return transactionPlugin(transaction).additionalRequiredCosignatories(transaction);
			}

		private:
			const EmbeddedTransactionPlugin& m_transactionPluginV1;
			const EmbeddedTransactionPlugin& m_transactionPluginV2;
		};

		class AggregateTransactionPlugin : public TransactionPlugin {
		public:
			AggregateTransactionPlugin(
					std::unique_ptr<TransactionPlugin>&& pTransactionPluginV1,
					std::unique_ptr<TransactionPlugin>&& pTransactionPluginV2)
					: m_pTransactionPluginV1(std::move(pTransactionPluginV1))
					, m_pTransactionPluginV2(std::move(pTransactionPluginV2))
					, m_pEmbeddedTransactionPlugin(std::make_unique<AggregateEmbededTransactionPlugin>(
							m_pTransactionPluginV1->embeddedPlugin(),
							m_pTransactionPluginV2->embeddedPlugin()))
			{}

		private:
			const auto& transactionPlugin(const Transaction& transaction) const {
				return *(1 == transaction.Version ? m_pTransactionPluginV1 : m_pTransactionPluginV2);
			}

		public:
			EntityType type() const override {
				return m_pTransactionPluginV2->type();
			}

			TransactionAttributes attributes() const override {
				return { 1, 2, m_pTransactionPluginV2->attributes().MaxLifetime };
			}

			bool isSizeValid(const Transaction& transaction) const override {
				return sizeof(Transaction) <= transaction.Size && transactionPlugin(transaction).isSizeValid(transaction);
			}

		public:
			void publish(
					const WeakEntityInfoT<Transaction>& transactionInfo,
					const PublishContext& context,
					NotificationSubscriber& sub) const override {
				transactionPlugin(transactionInfo.entity()).publish(transactionInfo, context, sub);
			}

			uint32_t embeddedCount(const Transaction& transaction) const override {
				return transactionPlugin(transaction).embeddedCount(transaction);
			}

			RawBuffer dataBuffer(const Transaction& transaction) const override {
				return transactionPlugin(transaction).dataBuffer(transaction);
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction& transaction) const override {
				return transactionPlugin(transaction).merkleSupplementaryBuffers(transaction);
			}

			bool supportsTopLevel() const override {
				return true;
			}

			bool supportsEmbedding() const override {
				return true;
			}

			const EmbeddedTransactionPlugin& embeddedPlugin() const override {
				return *m_pEmbeddedTransactionPlugin;
			}

		private:
			std::unique_ptr<TransactionPlugin> m_pTransactionPluginV1;
			std::unique_ptr<TransactionPlugin> m_pTransactionPluginV2;
			std::unique_ptr<EmbeddedTransactionPlugin> m_pEmbeddedTransactionPlugin;
		};

		DEFINE_TRANSACTION_PLUGIN_FACTORY(VotingKeyLinkV1, Default, Publish)

		std::unique_ptr<TransactionPlugin> CreateVotingKeyLinkV2TransactionPlugin() {
			using Factory = TransactionPluginFactory<TransactionPluginFactoryOptions::Default>;
			return Factory::Create<VotingKeyLinkTransaction, EmbeddedVotingKeyLinkTransaction>(
					Publish<VotingKeyLinkTransaction>,
					Publish<EmbeddedVotingKeyLinkTransaction>);
		}
	}

	std::unique_ptr<TransactionPlugin> CreateVotingKeyLinkTransactionPlugin() {
		return std::make_unique<AggregateTransactionPlugin>(
				CreateVotingKeyLinkV1TransactionPlugin(),
				CreateVotingKeyLinkV2TransactionPlugin());
	}
}}
