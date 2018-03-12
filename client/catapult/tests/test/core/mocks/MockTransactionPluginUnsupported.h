#pragma once
#include "MockTransaction.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace mocks {

	/// An unsupported mock transaction plugin.
	class MockTransactionPluginUnsupported : public model::TransactionPlugin {
	public:
		model::EntityType type() const override {
			return mocks::MockTransaction::Entity_Type;
		}

		uint64_t calculateRealSize(const model::Transaction&) const override {
			CATAPULT_THROW_RUNTIME_ERROR("calculateRealSize - not implemented in mock");
		}

		void publish(const model::WeakEntityInfoT<model::Transaction>&, model::NotificationSubscriber&) const override {
			CATAPULT_THROW_RUNTIME_ERROR("publish - not implemented in mock");
		}

		RawBuffer dataBuffer(const model::Transaction&) const override {
			CATAPULT_THROW_RUNTIME_ERROR("dataBuffer - not implemented in mock");
		}

		std::vector<RawBuffer> merkleSupplementaryBuffers(const model::Transaction&) const override {
			CATAPULT_THROW_RUNTIME_ERROR("merkleSupplementaryBuffers - not implemented in mock");
		}

		bool supportsEmbedding() const override {
			return false;
		}

		const model::EmbeddedTransactionPlugin& embeddedPlugin() const override {
			CATAPULT_THROW_RUNTIME_ERROR("embeddedPlugin - not implemented in mock");
		}
	};
}}
