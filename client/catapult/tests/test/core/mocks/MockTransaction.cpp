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

#include "MockTransaction.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/model/Address.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/nodeps/Random.h"

using namespace catapult::model;

namespace catapult { namespace mocks {

	namespace {
		template<typename TMockTransaction>
		std::unique_ptr<TMockTransaction> CreateMockTransactionT(uint16_t dataSize) {
			uint32_t entitySize = sizeof(TMockTransaction) + dataSize;
			auto pTransaction = utils::MakeUniqueWithSize<TMockTransaction>(entitySize);

			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pTransaction.get()), entitySize });
			pTransaction->Size = entitySize;
			pTransaction->Type = MockTransaction::Entity_Type;
			pTransaction->Data.Size = dataSize;
			return pTransaction;
		}
	}

	std::unique_ptr<MockTransaction> CreateMockTransaction(uint16_t dataSize) {
		return CreateMockTransactionT<MockTransaction>(dataSize);
	}

	std::unique_ptr<EmbeddedMockTransaction> CreateEmbeddedMockTransaction(uint16_t dataSize) {
		return CreateMockTransactionT<EmbeddedMockTransaction>(dataSize);
	}

	std::unique_ptr<mocks::MockTransaction> CreateTransactionWithFeeAndTransfers(Amount fee, const test::BalanceTransfers& transfers) {
		auto pTransaction = mocks::CreateMockTransaction(static_cast<uint16_t>(transfers.size() * sizeof(Mosaic)));
		pTransaction->Fee = fee;
		pTransaction->Version = 0;

		auto pTransfer = reinterpret_cast<Mosaic*>(pTransaction->DataPtr());
		for (const auto& transfer : transfers) {
			*pTransfer = transfer;
			++pTransfer;
		}

		return pTransaction;
	}

	std::unique_ptr<MockTransaction> CreateMockTransactionWithSignerAndRecipient(const Key& signer, const Key& recipient) {
		auto pTransaction = CreateMockTransactionT<MockTransaction>(0);
		pTransaction->Signer = signer;
		pTransaction->Recipient = recipient;
		pTransaction->Version = MakeVersion(NetworkIdentifier::Mijin_Test, 1);
		return pTransaction;
	}

	bool IsPluginOptionFlagSet(PluginOptionFlags options, PluginOptionFlags flag) {
		return utils::to_underlying_type(flag) == (utils::to_underlying_type(options) & utils::to_underlying_type(flag));
	}

	namespace {
		template<typename TTransaction>
		void Publish(
				const TTransaction& mockTransaction,
				const PublisherContext& publisherContext,
				PluginOptionFlags options,
				NotificationSubscriber& sub) {
			sub.notify(AccountPublicKeyNotification(mockTransaction.Recipient));

			if (IsPluginOptionFlagSet(options, PluginOptionFlags::Publish_Custom_Notifications)) {
				sub.notify(test::CreateNotification(Mock_Observer_1_Notification));
				sub.notify(test::CreateNotification(Mock_Validator_1_Notification));
				sub.notify(test::CreateNotification(Mock_All_1_Notification));
				sub.notify(test::CreateNotification(Mock_Observer_2_Notification));
				sub.notify(test::CreateNotification(Mock_Validator_2_Notification));
				sub.notify(test::CreateNotification(Mock_All_2_Notification));
			}

			if (!IsPluginOptionFlagSet(options, PluginOptionFlags::Publish_Transfers))
				return;

			auto pMosaics = reinterpret_cast<const UnresolvedMosaic*>(mockTransaction.DataPtr());
			for (auto i = 0u; i < mockTransaction.Data.Size / sizeof(UnresolvedMosaic); ++i) {
				const auto& sender = mockTransaction.Signer;
				auto recipient = PublicKeyToAddress(mockTransaction.Recipient, NetworkIdentifier::Mijin_Test);
				sub.notify(BalanceTransferNotification(
						sender,
						publisherContext.resolve(extensions::CopyToUnresolvedAddress(recipient)),
						publisherContext.resolve(pMosaics[i].MosaicId),
						pMosaics[i].Amount));
			}
		}

		template<typename TTransaction, typename TDerivedTransaction, typename TPlugin>
		class MockTransactionPluginT : public TPlugin {
		public:
			explicit MockTransactionPluginT(EntityType type, PluginOptionFlags options)
					: m_type(type)
					, m_options(options)
			{}

		public:
			EntityType type() const override {
				return m_type;
			}

			uint64_t calculateRealSize(const TTransaction& transaction) const override {
				return TDerivedTransaction::CalculateRealSize(static_cast<const TDerivedTransaction&>(transaction));
			}

		private:
			EntityType m_type;
			PluginOptionFlags m_options;
		};

		class EmbeddedMockTransactionPlugin
				: public MockTransactionPluginT<EmbeddedTransaction, EmbeddedMockTransaction, EmbeddedTransactionPlugin> {
		public:
			explicit EmbeddedMockTransactionPlugin(EntityType type, PluginOptionFlags options)
					: MockTransactionPluginT<EmbeddedTransaction, EmbeddedMockTransaction, EmbeddedTransactionPlugin>(type, options)
					, m_options(options)
			{}

		public:
			void publish(
					const EmbeddedTransaction& transaction,
					const PublisherContext& publisherContext,
					NotificationSubscriber& sub) const override {
				Publish(static_cast<const EmbeddedMockTransaction&>(transaction), publisherContext, m_options, sub);
			}

		private:
			PluginOptionFlags m_options;
		};

		class MockTransactionPlugin : public MockTransactionPluginT<Transaction, MockTransaction, TransactionPlugin> {
		public:
			explicit MockTransactionPlugin(EntityType type, PluginOptionFlags options)
					: MockTransactionPluginT<Transaction, MockTransaction, TransactionPlugin>(type, options)
					, m_options(options) {
				if (IsPluginOptionFlagSet(m_options, PluginOptionFlags::Not_Embeddable))
					return;

				m_pEmbeddedTransactionPlugin = std::make_unique<EmbeddedMockTransactionPlugin>(type, options);
			}

		public:
			void publish(
					const WeakEntityInfoT<Transaction>& transactionInfo,
					const PublisherContext& publisherContext,
					NotificationSubscriber& sub) const override {
				Publish(static_cast<const MockTransaction&>(transactionInfo.entity()), publisherContext, m_options, sub);

				// raise a custom notification that includes the provided hash
				// (this allows other tests to verify that the appropriate hash was passed down)
				if (IsPluginOptionFlagSet(m_options, PluginOptionFlags::Publish_Custom_Notifications))
					sub.notify(HashNotification(transactionInfo.hash()));
			}

			RawBuffer dataBuffer(const Transaction& transaction) const override {
				if (IsPluginOptionFlagSet(m_options, PluginOptionFlags::Custom_Buffers)) {
					// return only the mock transaction data payload
					// (returning a non-standard dataBuffer allows tests to ensure that dataBuffer() is being called)
					const auto& mockTransaction = static_cast<const MockTransaction&>(transaction);
					return { reinterpret_cast<const uint8_t*>(&mockTransaction + 1), mockTransaction.Data.Size };
				} else {
					// return entire transaction payload (this is the "correct" behavior)
					auto headerSize = VerifiableEntity::Header_Size;
					return { reinterpret_cast<const uint8_t*>(&transaction) + headerSize, transaction.Size - headerSize };
				}
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction&) const override {
				return {};
			}

			bool supportsEmbedding() const override {
				return !!m_pEmbeddedTransactionPlugin;
			}

			const EmbeddedTransactionPlugin& embeddedPlugin() const override {
				if (!m_pEmbeddedTransactionPlugin)
					CATAPULT_THROW_RUNTIME_ERROR("mock transaction is not embeddable");

				return *m_pEmbeddedTransactionPlugin;
			}

		private:
			PluginOptionFlags m_options;
			std::unique_ptr<EmbeddedTransactionPlugin> m_pEmbeddedTransactionPlugin;
		};
	}

	std::unique_ptr<TransactionPlugin> CreateMockTransactionPlugin(int type) {
		return std::make_unique<MockTransactionPlugin>(static_cast<EntityType>(type), PluginOptionFlags::Not_Embeddable);
	}

	std::unique_ptr<TransactionPlugin> CreateMockTransactionPlugin(PluginOptionFlags options) {
		// cast needed to avoid clang linker error
		return std::make_unique<MockTransactionPlugin>(static_cast<EntityType>(MockTransaction::Entity_Type), options);
	}

	std::unique_ptr<TransactionPlugin> CreateMockTransactionPlugin(int type, PluginOptionFlags options) {
		return std::make_unique<MockTransactionPlugin>(static_cast<EntityType>(type), options);
	}

	TransactionRegistry CreateDefaultTransactionRegistry(PluginOptionFlags options) {
		auto registry = TransactionRegistry();
		registry.registerPlugin(CreateMockTransactionPlugin(options));
		return registry;
	}
}}
