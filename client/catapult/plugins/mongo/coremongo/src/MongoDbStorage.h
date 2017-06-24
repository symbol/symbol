#pragma once
#include "MongoStorageConfiguration.h"
#include "catapult/io/BlockStorage.h"
#include "catapult/local/api/ChainScoreProvider.h"
#include "catapult/types.h"

namespace catapult { namespace mongo { namespace plugins { class MongoTransactionRegistry; } } }

namespace catapult { namespace mongo { namespace plugins {

	/// Mongodb backed block storage and state provider.
	class MongoDbStorage final : public io::LightBlockStorage, public local::api::ChainScoreProvider {
	public:
		/// Creates a mongodb storage around \a pConfig and \a pTransactionRegistry.
		explicit MongoDbStorage(
				const std::shared_ptr<MongoStorageConfiguration>& pConfig,
				const std::shared_ptr<const MongoTransactionRegistry>& pTransactionRegistry);

	public:
		Height chainHeight() const override;

	public:
		model::HashRange loadHashesFrom(Height height, size_t maxHashes) const override;

		void saveBlock(const model::BlockElement& blockElement) override;

		void dropBlocksAfter(Height height) override;

	public:
		void saveScore(const model::ChainScore& chainScore) override;

		model::ChainScore loadScore() const override;

	private:
		std::shared_ptr<MongoStorageConfiguration> m_pConfig;
		std::shared_ptr<const MongoTransactionRegistry> m_pTransactionRegistry;
		MongoDatabase m_database;
	};
}}}
