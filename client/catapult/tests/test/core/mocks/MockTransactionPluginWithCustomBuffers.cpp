#include "MockTransactionPluginWithCustomBuffers.h"
#include "MockTransaction.h"

namespace catapult { namespace mocks {

	RawBuffer ExtractBuffer(const OffsetRange& range, const void* pVoid) {
		const auto* pData = reinterpret_cast<const uint8_t*>(pVoid);
		return { pData + range.Start, range.End - range.Start };
	}

	namespace {
		class MockTransactionPluginWithCustomBuffers : public model::TransactionPlugin {
		public:
			explicit MockTransactionPluginWithCustomBuffers(
					const OffsetRange& dataRange,
					const std::vector<OffsetRange>& supplementalRanges)
					: m_dataRange(dataRange)
					, m_supplementalRanges(supplementalRanges)
			{}

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

			RawBuffer dataBuffer(const model::Transaction& transaction) const override {
				return ExtractBuffer(m_dataRange, &transaction);
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const model::Transaction& transaction) const override {
				std::vector<RawBuffer> supplementaryBuffers;
				for (const auto& range : m_supplementalRanges)
					supplementaryBuffers.push_back(ExtractBuffer(range, &transaction));

				return supplementaryBuffers;
			}

			bool supportsEmbedding() const override {
				return false;
			}

			const model::EmbeddedTransactionPlugin& embeddedPlugin() const override {
				CATAPULT_THROW_RUNTIME_ERROR("embeddedPlugin - not implemented in mock");
			}

		private:
			OffsetRange m_dataRange;
			std::vector<OffsetRange> m_supplementalRanges;
		};
	}

	std::unique_ptr<model::TransactionPlugin> CreateMockTransactionPluginWithCustomBuffers(
			const OffsetRange& dataRange,
			const std::vector<OffsetRange>& supplementalRanges) {
		return std::make_unique<MockTransactionPluginWithCustomBuffers>(dataRange, supplementalRanges);
	}
}}
