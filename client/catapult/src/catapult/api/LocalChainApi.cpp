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

#include "LocalChainApi.h"
#include "catapult/io/BlockStorageCache.h"

namespace catapult { namespace api {

	namespace {
		catapult_api_error CreateHeightException(const char* message, Height height) {
			return catapult_api_error(message) << exception_detail::Make<Height>::From(height);
		}

		class LocalChainApi : public ChainApi {
		public:
			LocalChainApi(
					const io::BlockStorageCache& storage,
					const model::ChainScoreSupplier& chainScoreSupplier,
					const supplier<Height>& finalizedHeightSupplier)
					: m_storage(storage)
					, m_chainScoreSupplier(chainScoreSupplier)
					, m_finalizedHeightSupplier(finalizedHeightSupplier)
			{}

		public:
			thread::future<ChainStatistics> chainStatistics() const override {
				auto chainStatistics = ChainStatistics();
				chainStatistics.Height = m_storage.view().chainHeight();
				chainStatistics.FinalizedHeight = m_finalizedHeightSupplier();
				chainStatistics.Score = m_chainScoreSupplier();
				return thread::make_ready_future(std::move(chainStatistics));
			}

			thread::future<model::HashRange> hashesFrom(Height height, uint32_t maxHashes) const override {
				auto hashes = m_storage.view().loadHashesFrom(height, maxHashes);
				if (hashes.empty()) {
					auto exception = CreateHeightException("unable to get hashes from height", height);
					return thread::make_exceptional_future<model::HashRange>(exception);
				}

				return thread::make_ready_future(std::move(hashes));
			}

		private:
			const io::BlockStorageCache& m_storage;
			model::ChainScoreSupplier m_chainScoreSupplier;
			supplier<Height> m_finalizedHeightSupplier;
		};
	}

	std::unique_ptr<ChainApi> CreateLocalChainApi(
			const io::BlockStorageCache& storage,
			const model::ChainScoreSupplier& chainScoreSupplier,
			const supplier<Height>& finalizedHeightSupplier) {
		return std::make_unique<LocalChainApi>(storage, chainScoreSupplier, finalizedHeightSupplier);
	}
}}
