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
			MockTransactionPluginWithCustomBuffers(const OffsetRange& dataRange, const std::vector<OffsetRange>& supplementalRanges)
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
