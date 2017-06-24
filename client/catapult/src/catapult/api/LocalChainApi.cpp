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
					const ChainScoreSupplier& chainScoreSupplier,
					uint32_t maxHashes)
					: m_storage(storage)
					, m_chainScoreSupplier(chainScoreSupplier)
					, m_maxHashes(maxHashes)
			{}

		public:
			thread::future<ChainInfo> chainInfo() const override {
				auto info = ChainInfo();
				info.Height = m_storage.view().chainHeight();
				info.Score = m_chainScoreSupplier();
				return thread::make_ready_future(std::move(info));
			}

			thread::future<model::HashRange> hashesFrom(Height height) const override {
				auto hashes = m_storage.view().loadHashesFrom(height, m_maxHashes);
				if (hashes.empty()) {
					auto exception = CreateHeightException("unable to get hashes from height", height);
					return thread::make_exceptional_future<model::HashRange>(exception);
				}

				return thread::make_ready_future(std::move(hashes));
			}

		private:
			const io::BlockStorageCache& m_storage;
			ChainScoreSupplier m_chainScoreSupplier;
			uint32_t m_maxHashes;
		};
	}

	std::unique_ptr<ChainApi> CreateLocalChainApi(
			const io::BlockStorageCache& storage,
			const ChainScoreSupplier& chainScoreSupplier,
			uint32_t maxHashes) {
		return std::make_unique<LocalChainApi>(storage, chainScoreSupplier, maxHashes);
	}
}}
