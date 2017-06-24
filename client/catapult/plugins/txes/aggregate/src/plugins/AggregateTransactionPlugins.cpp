#include "AggregateTransactionPlugins.h"
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

		class AggregateTransactionPlugin : public TransactionPlugin {
		public:
			explicit AggregateTransactionPlugin(const TransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			EntityType type() const override {
				return EntityType::Aggregate;
			}

			uint64_t calculateRealSize(const Transaction& transaction) const override {
				// if size is valid, the real size is the transaction size
				// if size is invalid, return a size that can never be correct (transaction size is uint32_t)
				return IsSizeValid(CastToDerivedType(transaction), m_transactionRegistry)
						? transaction.Size
						: std::numeric_limits<uint64_t>::max();
			}

			void publish(const WeakEntityInfoT<Transaction>& transactionInfo, NotificationSubscriber& sub) const override {
				const auto& aggregate = CastToDerivedType(transactionInfo.entity());

				// publish aggregate notifications
				auto numCosignatures = aggregate.CosignaturesCount();
				sub.notify(AggregateCosignaturesNotification(
						aggregate.Signer,
						static_cast<uint32_t>(std::distance(aggregate.Transactions().cbegin(), aggregate.Transactions().cend())),
						aggregate.TransactionsPtr(),
						numCosignatures,
						aggregate.CosignaturesPtr()));

				// publish all sub-transaction information
				for (const auto& subTransaction : aggregate.Transactions()) {
					// - signers
					sub.notify(AccountPublicKeyNotification(subTransaction.Signer));

					// - generic sub-transaction notification
					sub.notify(AggregateEmbeddedTransactionNotification(
							aggregate.Signer,
							subTransaction,
							numCosignatures,
							aggregate.CosignaturesPtr()));

					// - specific sub-transaction notifications
					//   (calculateRealSize would have failed if plugin is unknown or not embeddable)
					const auto& plugin = m_transactionRegistry.findPlugin(subTransaction.Type)->embeddedPlugin();
					plugin.publish(subTransaction, sub);
				}

				// publish all cosigner information
				const auto* pCosignature = aggregate.CosignaturesPtr();
				for (auto i = 0u; i < numCosignatures; ++i) {
					// - notice that all valid cosigners must have been observed previously as part of either
					//   (1) sub-transaction execution or (2) composite account setup
					// - require the cosigners to sign the aggregate indirectly via the hash of its data
					sub.notify(SignatureNotification(pCosignature->Signer, pCosignature->Signature, transactionInfo.hash()));
					++pCosignature;
				}
			}

			RawBuffer dataBuffer(const Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);

				auto headerSize = VerifiableEntity::Header_Size;
				return {
					reinterpret_cast<const uint8_t*>(&aggregate) + headerSize,
					sizeof(AggregateTransaction) - headerSize + aggregate.PayloadSize
				};
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction& transaction) const override {
				const auto& aggregate = CastToDerivedType(transaction);

				std::vector<RawBuffer> buffers;
				auto numCosignatures = aggregate.CosignaturesCount();
				const auto* pCosignature = aggregate.CosignaturesPtr();
				for (auto i = 0u; i < numCosignatures; ++i) {
					buffers.push_back(pCosignature->Signer);
					++pCosignature;
				}

				return buffers;
			}

			bool supportsEmbedding() const override {
				return false;
			}

			const EmbeddedTransactionPlugin& embeddedPlugin() const override {
				CATAPULT_THROW_RUNTIME_ERROR("aggregate transaction is not embeddable");
			}

		private:
			const TransactionRegistry& m_transactionRegistry;
		};
	}

	std::unique_ptr<TransactionPlugin> CreateAggregateTransactionPlugin(const TransactionRegistry& transactionRegistry) {
		return std::make_unique<AggregateTransactionPlugin>(transactionRegistry);
	}
}}
