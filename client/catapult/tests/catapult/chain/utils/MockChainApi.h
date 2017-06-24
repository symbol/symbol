#pragma once
#include "catapult/api/RemoteChainApi.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/HashTestUtils.h"
#include <map>
#include <thread>

namespace catapult { namespace mocks {

	/// A mock chain api that can be configured to return predefined data for requests, capture function parameters
	/// and throw exceptions at specified entry points.
	class MockChainApi : public api::RemoteChainApi {
	public:
		enum class EntryPoint {
			Chain_Info,
			Hashes_From,
			Last_Block,
			Block_At,
			Blocks_From,
			None
		};

	public:
		/// Creates a mock chain api around a chain \a score, a last block (\a pLastBlock) and a range of \a hashes.
		MockChainApi(
				model::ChainScore score,
				std::shared_ptr<model::Block>&& pLastBlock,
				const model::HashRange& hashes)
				: m_score(score)
				, m_errorEntryPoint(EntryPoint::None)
				, m_hashes(model::HashRange::CopyRange(hashes))
				, m_numBlocksPerBlocksFromRequest({ 2 }) {
			m_blocks.emplace(Height(0), std::move(pLastBlock));
		}

		/// Creates a mock chain api around a chain \a score, a last block (\a pLastBlock) and the number of
		/// hahes (\a numHashesToReturn) to return in a hashes-from request.
		MockChainApi(
				model::ChainScore score,
				std::shared_ptr<model::Block>&& pLastBlock,
				size_t numHashesToReturn = 0)
				: MockChainApi(score, std::move(pLastBlock), test::GenerateRandomHashes(numHashesToReturn))
		{}

	public:
		/// Sets the entry point where an exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Sets the \a delay that chain requests should wait before returning a response.
		void setDelay(const utils::TimeSpan& delay) {
			m_apiDelay = delay;
		}

		/// Adds a block (\a pBlock) to the block map.
		void addBlock(std::unique_ptr<model::Block>&& pBlock) {
			auto height = pBlock->Height;
			m_blocks.emplace(height, std::move(pBlock));
		}

		/// Returns the vector of heights that were passed to the block-at requests.
		const std::vector<Height>& blockAtRequests() const {
			return m_blockAtRequests;
		}

		/// Returns the vector of heights that were passed to the hashes-from requests.
		const std::vector<Height>& hashesFromRequests() const {
			return m_hashesFromRequests;
		}

		/// Returns the vector of height/blocks-from-options pairs that were passed to the blocks-from requests.
		const std::vector<std::pair<Height, const api::BlocksFromOptions>>& blocksFromRequests() const {
			return m_blocksFromRequests;
		}

		/// Sets the number of blocks (\a numBlocksPerBlocksFromRequest) to return for multiple blocks-from requests.
		/// \note The last value will be repeated indefinitely.
		void setNumBlocksPerBlocksFromRequest(const std::vector<uint32_t>& numBlocksPerBlocksFromRequest) {
			m_numBlocksPerBlocksFromRequest.clear();
			m_numBlocksPerBlocksFromRequest.insert(
					m_numBlocksPerBlocksFromRequest.end(),
					numBlocksPerBlocksFromRequest.cbegin(),
					numBlocksPerBlocksFromRequest.cend());
		}

	public:
		/// Returns the configured chain info, throws if the error entry point is set to Chain_Info.
		thread::future<api::ChainInfo> chainInfo() const override {
			if (shouldRaiseException(EntryPoint::Chain_Info))
				return CreateFutureException<api::ChainInfo>("chain info error has been set");

			auto info = api::ChainInfo();
			info.Height = chainHeight();
			info.Score = m_score;
			return CreateFutureResponse(std::move(info));
		}

		/// Returns the configured hashes from \a height, throws if the error entry point is set to Hashes_From.
		/// The \a height parameter is captured.
		thread::future<model::HashRange> hashesFrom(Height height) const override {
			m_hashesFromRequests.push_back(height);
			if (shouldRaiseException(EntryPoint::Hashes_From))
				return CreateFutureException<model::HashRange>("hashes from error has been set");

			return CreateFutureResponse(model::HashRange::CopyRange(m_hashes));
		}

		/// Returns the configured last block, throws if the error entry point is set to Last_Block.
		thread::future<std::shared_ptr<const model::Block>> blockLast() const override {
			if (shouldRaiseException(EntryPoint::Last_Block))
				return CreateFutureException<std::shared_ptr<const model::Block>>("last block error has been set");

			return blockAt(Height(0));
		}

		/// Returns the configured block at \a height, throws if the error entry point is set to Block_At.
		/// The \a height parameter is captured.
		thread::future<std::shared_ptr<const model::Block>> blockAt(Height height) const override {
			m_blockAtRequests.push_back(height);
			if (Height(0) != height && shouldRaiseException(EntryPoint::Block_At))
				return CreateFutureException<std::shared_ptr<const model::Block>>("block at error has been set");

			auto iter = m_blocks.find(height);
			return CreateFutureResponse(std::shared_ptr<const model::Block>(test::CopyBlock(*iter->second)));
		}

		/// Returns a range of the configured blocks, throws if the error entry point is set to Blocks_From.
		/// The \a height and the blocks-from-options (\a options) parameters are captured.
		thread::future<model::BlockRange> blocksFrom (
				Height height,
				const api::BlocksFromOptions& options) const override {
			m_blocksFromRequests.push_back(std::make_pair(height, options));
			if (shouldRaiseException(EntryPoint::Blocks_From))
				return CreateFutureException<model::BlockRange>("blocks from error has been set");

			// use the next configured numBlocks value and pop it if it isn't the last one (the last value is used indefinitely)
			auto numBlocks = m_numBlocksPerBlocksFromRequest.front();
			if (m_numBlocksPerBlocksFromRequest.size() > 1)
				m_numBlocksPerBlocksFromRequest.pop_front();

			return CreateFutureResponse(createRange(height, numBlocks));
		}

	private:
		bool shouldRaiseException(EntryPoint entryPoint) const {
			return m_errorEntryPoint == entryPoint;
		}

		Height chainHeight() const {
			auto lastBlockIter = m_blocks.find(Height(0));
			return m_blocks.cend() == lastBlockIter ? Height(0) : lastBlockIter->second->Height;
		}

		template<typename T>
		static thread::future<T> CreateFutureException(const char* message) {
			return thread::make_exceptional_future<T>(catapult_runtime_error(message));
		}

		model::BlockRange createRange(Height startHeight, size_t numBlocks) const {
			std::vector<std::unique_ptr<const model::Block>> blocks;
			std::vector<const model::Block*> rawBlocks;
			for (auto i = 0u; i < numBlocks; ++i) {
				blocks.push_back(test::GenerateVerifiableBlockAtHeight(startHeight + Height(i)));
				rawBlocks.push_back(blocks[i].get());
			}

			return test::CreateEntityRange(rawBlocks);
		}

		template<typename T>
		thread::future<T> CreateFutureResponse(T&& value) const {
			// if no delay is specified, resolve the future immediately
			if (utils::TimeSpan() == m_apiDelay)
				return thread::make_ready_future(std::move(value));

			// if a delay is specified, delay the future a little bit
			thread::promise<T> promise;
			auto future = promise.get_future();
			std::thread([promise = std::move(promise), value = std::move(value), delay = m_apiDelay.millis()]() mutable {
				test::Sleep(static_cast<uint32_t>(delay));
				promise.set_value(std::move(value));
			}).detach();
			return future;
		}

	private:
		model::ChainScore m_score;
		EntryPoint m_errorEntryPoint;
		model::HashRange m_hashes;
		std::map<Height, std::shared_ptr<model::Block>> m_blocks;
		mutable std::vector<Height> m_blockAtRequests;
		mutable std::vector<Height> m_hashesFromRequests;
		mutable std::vector<std::pair<Height, const api::BlocksFromOptions>> m_blocksFromRequests;
		utils::TimeSpan m_apiDelay;
		mutable std::list<uint32_t> m_numBlocksPerBlocksFromRequest;
	};
}}
