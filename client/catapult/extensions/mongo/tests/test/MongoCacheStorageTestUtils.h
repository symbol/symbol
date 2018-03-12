#pragma once
#include "MongoTestUtils.h"
#include "mongo/src/MongoStorageContext.h"

namespace catapult { namespace test {

	/// Base class that provides shared utils for mongo cache storage test suites.
	template<typename TTraits>
	class MongoCacheStorageTestUtils {
	private:
		using CacheType = typename TTraits::CacheType;
		using ElementType = typename TTraits::ModelType;

	protected:
		class CacheStorageWrapper : public PrepareDatabaseMixin {
		public:
			CacheStorageWrapper()
					: m_pMongoContext(CreateDefaultMongoStorageContext(DatabaseName()))
					, m_pCacheStorage(TTraits::CreateCacheStorage(
							m_pMongoContext->createDatabaseConnection(),
							m_pMongoContext->bulkWriter(),
							TTraits::Network_Id))
			{}

		public:
			mongo::ExternalCacheStorage& get() {
				return *m_pCacheStorage;
			}

		private:
			std::unique_ptr<mongo::MongoStorageContext> m_pMongoContext;
			std::unique_ptr<mongo::ExternalCacheStorage> m_pCacheStorage;
		};

	protected:
		static const auto& GetDelta(const cache::CatapultCacheDelta& delta) {
			return delta.template sub<CacheType>();
		}

		static int64_t GetCollectionSize() {
			auto connection = CreateDbConnection();
			auto collection = connection[DatabaseName()][TTraits::Collection_Name];
			return collection.count({});
		}

		static void AssertDbContents(const std::vector<ElementType>& elements, size_t numHiddenElements = 0) {
			auto connection = CreateDbConnection();
			auto database = connection[DatabaseName()];

			AssertCollectionSize(TTraits::Collection_Name, elements.size() + numHiddenElements);
			for (const auto& element : elements)
				AssertDbContains(database, element);
		}

	private:
		static void AssertDbContains(mongocxx::database& database, const ElementType& element) {
			auto filter = TTraits::GetFindFilter(element);
			auto matchedDocument = database[TTraits::Collection_Name].find_one(filter.view());
			ASSERT_TRUE(matchedDocument.is_initialized());

			TTraits::AssertEqual(element, matchedDocument.get().view());
		}
	};
}}
