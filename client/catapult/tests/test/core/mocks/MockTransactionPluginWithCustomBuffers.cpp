#include "MockTransactionPluginWithCustomBuffers.h"
#include "MockTransactionPluginUnsupported.h"

namespace catapult { namespace mocks {

	RawBuffer ExtractBuffer(const OffsetRange& range, const void* pVoid) {
		const auto* pData = reinterpret_cast<const uint8_t*>(pVoid);
		return { pData + range.Start, range.End - range.Start };
	}

	namespace {
		class MockTransactionPluginWithCustomBuffers : public MockTransactionPluginUnsupported {
		public:
			explicit MockTransactionPluginWithCustomBuffers(
					const OffsetRange& dataRange,
					const std::vector<OffsetRange>& supplementalRanges)
					: m_dataRange(dataRange)
					, m_supplementalRanges(supplementalRanges)
			{}

		public:
			RawBuffer dataBuffer(const model::Transaction& transaction) const override {
				return ExtractBuffer(m_dataRange, &transaction);
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const model::Transaction& transaction) const override {
				std::vector<RawBuffer> supplementaryBuffers;
				for (const auto& range : m_supplementalRanges)
					supplementaryBuffers.push_back(ExtractBuffer(range, &transaction));

				return supplementaryBuffers;
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
