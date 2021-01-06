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

			enum class Stage { Statistics, Hashes_First, Hashes_Next, Score_Local };

		public:
			CompareChainsContext(const api::ChainApi& local, const api::ChainApi& remote, const CompareChainsOptions& options)
					: m_local(local)
					, m_remote(remote)
					, m_options(options)
					, m_stage(Stage::Statistics)
			{}

		public:
			thread::future<CompareChainsResult> compare() {
				startNextCompare();
				return m_promise.get_future();
			}

		private:
			void startNextCompare() {
				switch (m_stage) {
				case Stage::Statistics:
					m_lowerBoundHeight = m_options.FinalizedHeightSupplier();
					m_startingHashesHeight = m_lowerBoundHeight;
					dispatch([](auto& context) { return context.compareChainStatistics(); });
					break;

				case Stage::Hashes_First:
				case Stage::Hashes_Next:
					dispatch([](auto& context) { return context.compareHashes(); });
					break;

				case Stage::Score_Local:
					// this will always terminate
					dispatch([](auto& context) { return context.compareLocalScore(); });
					break;
				}
			}

			void dispatch(ComparisonFunction nextFunc) {
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
					m_originalLocalScore = localChainStatistics.Score;
					m_localHeight = localChainStatistics.Height;
					m_remoteHeight = remoteChainStatistics.Height;

					m_upperBoundHeight = m_localHeight;
					m_stage = Stage::Hashes_First;
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
				auto startingHeight = m_startingHashesHeight;
				auto maxHashes = m_options.HashesPerBatch;

				CATAPULT_LOG(debug)
						<< "comparing hashes with local height " << m_localHeight
						<< ", starting height " << startingHeight
						<< ", max hashes " << maxHashes;

				return thread::when_all(m_local.hashesFrom(startingHeight, maxHashes), m_remote.hashesFrom(startingHeight, maxHashes))
					.then([pThis = shared_from_this()](auto&& aggregateFuture) {
						auto hashesFuture = aggregateFuture.get();
						const auto& localHashes = hashesFuture[0].get();
						const auto& remoteHashes = hashesFuture[1].get();
						return pThis->compareHashes(localHashes, remoteHashes);
					});
			}

			ChainComparisonCode compareHashes(const model::HashRange& localHashes, const model::HashRange& remoteHashes) {
				if (remoteHashes.size() > m_options.HashesPerBatch || 0 == remoteHashes.size())
					return ChainComparisonCode::Remote_Returned_Too_Many_Hashes;

				// at least the first compared block should be the same; if not, the remote is a liar or on a fork
				auto firstDifferenceIndex = FindFirstDifferenceIndex(localHashes, remoteHashes);
				if (isProcessingFirstBatchOfHashes() && 0 == firstDifferenceIndex)
					return ChainComparisonCode::Remote_Is_Forked;

				// need to use min because remote is allowed to return [1, m_options.HashesPerBatch] hashes
				auto commonBlockHeight = m_startingHashesHeight + Height(firstDifferenceIndex - 1);
				auto localHeightDerivedFromHashes = m_startingHashesHeight + Height(std::min(localHashes.size(), remoteHashes.size()) - 1);

				if (0 == firstDifferenceIndex) {
					// search previous hashes for first common block
					m_upperBoundHeight = m_startingHashesHeight;
					return tryContinue(Height((m_lowerBoundHeight + m_startingHashesHeight).unwrap() / 2));
				}

				if (remoteHashes.size() == firstDifferenceIndex) {
					if (localHeightDerivedFromHashes >= m_localHeight) {
						if (localHeightDerivedFromHashes < m_remoteHeight) {
							CATAPULT_LOG(debug)
									<< "preparing final comparison with local height " << m_localHeight
									<< ", derived local height " << localHeightDerivedFromHashes
									<< ", remote height " << m_remoteHeight;
							return tryContinue(localHeightDerivedFromHashes - Height(1));
						}

						m_stage = Stage::Score_Local;
						return Incomplete_Chain_Comparison_Code;
					}

					// search next hashes for first difference block
					m_lowerBoundHeight = m_startingHashesHeight;
					return tryContinue(Height((m_startingHashesHeight + m_upperBoundHeight).unwrap() / 2));
				}

				m_commonBlockHeight = commonBlockHeight;
				if (localHeightDerivedFromHashes > m_localHeight)
					m_localHeight = localHeightDerivedFromHashes;

				return ChainComparisonCode::Remote_Is_Not_Synced;
			}

			bool isProcessingFirstBatchOfHashes() const {
				return Stage::Hashes_First == m_stage;
			}

			ChainComparisonCode tryContinue(Height nextStartingHashesHeight) {
				if (m_startingHashesHeight == nextStartingHashesHeight)
					return ChainComparisonCode::Remote_Lied_About_Chain_Score;

				m_startingHashesHeight = nextStartingHashesHeight;
				m_stage = Stage::Hashes_Next;
				return Incomplete_Chain_Comparison_Code;
			}

			thread::future<ChainComparisonCode> compareLocalScore() {
				return m_local.chainStatistics().then([pThis = shared_from_this()](auto&& chainStatisticsFuture) {
					// if local score increased, don't punish remote
					auto currentLocalScore = chainStatisticsFuture.get().Score;
					if (currentLocalScore > pThis->m_originalLocalScore) {
						CATAPULT_LOG(debug)
								<< "local node score updated during compare chains from " << pThis->m_originalLocalScore
								<< " to " << currentLocalScore;
						return ChainComparisonCode::Local_Score_Updated;
					}

					return ChainComparisonCode::Remote_Lied_About_Chain_Score;
				});
			}

		private:
			const api::ChainApi& m_local;
			const api::ChainApi& m_remote;
			CompareChainsOptions m_options;
			Stage m_stage;

			Height m_lowerBoundHeight;
			Height m_upperBoundHeight;
			Height m_startingHashesHeight;

			thread::promise<CompareChainsResult> m_promise;

			model::ChainScore m_originalLocalScore; // original local score

			Height m_localHeight;
			Height m_remoteHeight;
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
