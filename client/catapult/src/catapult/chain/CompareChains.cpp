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

#include "CompareChains.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/Casting.h"
#include <iostream>

namespace catapult { namespace chain {

	namespace {
		constexpr auto Incomplete_Chain_Comparison_Code = static_cast<ChainComparisonCode>(std::numeric_limits<uint32_t>::max());

		class CompareChainsContext : public std::enable_shared_from_this<CompareChainsContext> {
		private:
			using ComparisonFunction = thread::future<ChainComparisonCode> (*)(CompareChainsContext& context);

		public:
			CompareChainsContext(const api::ChainApi& local, const api::ChainApi& remote, const CompareChainsOptions& options)
					: m_local(local)
					, m_remote(remote)
					, m_options(options)
					, m_nextFutureId(0)
			{}

		public:
			thread::future<CompareChainsResult> compare() {
				startNextCompare();
				return m_promise.get_future();
			}

		private:
			void startNextCompare() {
				ComparisonFunction nextFunc;
				if (0 == m_nextFutureId++) {
					nextFunc = [](auto& context) { return context.compareChainStatistics(); };
					m_nextHashesHeight = m_options.FinalizedHeightSupplier();
				} else {
					nextFunc = [](auto& context) { return context.compareHashes(); };
				}

				nextFunc(*this).then([pThis = shared_from_this()](auto&& future) {
					if (pThis->isFutureChainComplete(future))
						return;

					pThis->startNextCompare();
				});
			}

			bool isFutureChainComplete(thread::future<ChainComparisonCode>& future) {
				try {
					auto code = future.get();
					if (Incomplete_Chain_Comparison_Code == code)
						return false;

					auto forkDepth = (m_localHeight - m_commonBlockHeight).unwrap();
					auto result = ChainComparisonCode::Remote_Is_Not_Synced == code
							? CompareChainsResult{ code, m_commonBlockHeight, forkDepth }
							: CompareChainsResult{ code, Height(static_cast<Height::ValueType>(-1)), 0 };
					m_promise.set_value(std::move(result));
					return true;
				} catch (...) {
					m_promise.set_exception(std::current_exception());
					return true;
				}
			}

			thread::future<ChainComparisonCode> compareChainStatistics() {
				return thread::when_all(m_local.chainStatistics(), m_remote.chainStatistics()).then([pThis = shared_from_this()](
						auto&& aggregateFuture) {
					auto chainStatisticsFutures = aggregateFuture.get();
					auto localChainStatistics = chainStatisticsFutures[0].get();
					auto remoteChainStatistics = chainStatisticsFutures[1].get();
					return pThis->compareChainStatistics(localChainStatistics, remoteChainStatistics);
				});
			}

			ChainComparisonCode compareChainStatistics(
					const api::ChainStatistics& localChainStatistics,
					const api::ChainStatistics& remoteChainStatistics) {
				if (isRemoteTooFarBehind(remoteChainStatistics.Height))
					return ChainComparisonCode::Remote_Is_Too_Far_Behind;

				const auto& localScore = localChainStatistics.Score;
				const auto& remoteScore = remoteChainStatistics.Score;
				CATAPULT_LOG_LEVEL(localScore == remoteScore ? utils::LogLevel::trace : utils::LogLevel::debug)
						<< "comparing chain scores: " << localScore << " (local) vs " << remoteScore << " (remote)";

				if (remoteScore > localScore) {
					m_localHeight = localChainStatistics.Height;
					return Incomplete_Chain_Comparison_Code;
				}

				return localScore == remoteScore
						? ChainComparisonCode::Remote_Reported_Equal_Chain_Score
						: ChainComparisonCode::Remote_Reported_Lower_Chain_Score;
			}

			bool isRemoteTooFarBehind(Height remoteHeight) const {
				return remoteHeight <= m_options.FinalizedHeightSupplier();
			}

			thread::future<ChainComparisonCode> compareHashes() {
				auto startingHeight = m_nextHashesHeight;
				auto maxHashes = m_options.HashesPerBatch;

				return thread::when_all(m_local.hashesFrom(startingHeight, maxHashes), m_remote.hashesFrom(startingHeight, maxHashes))
					.then([pThis = shared_from_this(), startingHeight](auto&& aggregateFuture) {
						auto hashesFuture = aggregateFuture.get();
						const auto& localHashes = hashesFuture[0].get();
						const auto& remoteHashes = hashesFuture[1].get();
						return pThis->compareHashes(startingHeight, localHashes, remoteHashes);
					});
			}

			ChainComparisonCode compareHashes(
					Height startingHeight,
					const model::HashRange& localHashes,
					const model::HashRange& remoteHashes) {
				if (remoteHashes.size() > m_options.HashesPerBatch)
					return ChainComparisonCode::Remote_Returned_Too_Many_Hashes;

				// at least the first compared block should be the same; if not, the remote is a liar or on a fork
				auto firstDifferenceIndex = FindFirstDifferenceIndex(localHashes, remoteHashes);
				if (isProcessingFirstBatchOfHashes() && 0 == firstDifferenceIndex)
					return ChainComparisonCode::Remote_Is_Forked;

				auto commonBlockHeight = startingHeight + Height(firstDifferenceIndex - 1);
				auto localHeightDerivedFromHashes = startingHeight + Height(localHashes.size() - 1);

				// if all the hashes match, check if there is another batch that needs to be processed
				if (remoteHashes.size() == firstDifferenceIndex) {
					auto code = compareHashesWhenAllRemoteHashesMatch(localHeightDerivedFromHashes, remoteHashes.size());
					if (ChainComparisonCode::Remote_Is_Not_Synced != code)
						return code;
				}

				m_commonBlockHeight = commonBlockHeight;
				m_localHeight = localHeightDerivedFromHashes;
				return ChainComparisonCode::Remote_Is_Not_Synced;
			}

			bool isProcessingFirstBatchOfHashes() const {
				return 2 == m_nextFutureId;
			}

			ChainComparisonCode compareHashesWhenAllRemoteHashesMatch(Height localHeightDerivedFromHashes, size_t numRemoteHashes) {
				// allow an additional batch only if the remote returned a full batch of hashes and there are unprocessed local hashes
				if (m_options.HashesPerBatch == numRemoteHashes) {
					if (localHeightDerivedFromHashes >= m_localHeight) {
						// if all local hashes have been checked and the hashes end on a boundary, allow a sync with the remote since
						// it reported a higher score
						return ChainComparisonCode::Remote_Is_Not_Synced;
					} else {
						// pull the next batch from the remote
						m_nextHashesHeight = localHeightDerivedFromHashes + Height(1);
						return Incomplete_Chain_Comparison_Code;
					}
				}

				// if all hashes match and the local height is unchanged, the remote lied because it can't have a higher score
				return m_localHeight < localHeightDerivedFromHashes
						? ChainComparisonCode::Local_Height_Updated
						: ChainComparisonCode::Remote_Lied_About_Chain_Score;
			}

		private:
			const api::ChainApi& m_local;
			const api::ChainApi& m_remote;
			CompareChainsOptions m_options;

			size_t m_nextFutureId;
			Height m_nextHashesHeight;

			thread::promise<CompareChainsResult> m_promise;

			Height m_localHeight;
			Height m_commonBlockHeight;
		};
	}

	thread::future<CompareChainsResult> CompareChains(
			const api::ChainApi& local,
			const api::ChainApi& remote,
			const CompareChainsOptions& options) {
		auto pContext = std::make_shared<CompareChainsContext>(local, remote, options);
		return pContext->compare();
	}
}}
