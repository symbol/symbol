#include "src/storages/MongoMosaicCacheStorage.h"
#include "src/mappers/MosaicDescriptor.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MongoHistoricalCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/namespace/tests/test/MosaicCacheTestUtils.h"
#include "plugins/txes/namespace/tests/test/MosaicTestUtils.h"
#include "tests/test/NamespaceMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoMosaicCacheStorageTests

	namespace {
		struct MosaicCacheTraits {
			using CacheType = cache::MosaicCache;
			using ModelType = MosaicDescriptor;

			static constexpr auto Collection_Name = "mosaics";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoMosaicCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::MosaicCacheFactory::Create();
			}

			static MosaicDescriptor GenerateRandomElement(uint32_t id, uint32_t index, bool isActive) {
				return CreateElement(test::GenerateRandomData<Key_Size>(), id, index, isActive);
			}

			static MosaicDescriptor CreateElement(const Key& key, uint32_t id, uint32_t index, bool isActive) {
				auto pEntry = test::CreateMosaicEntry(NamespaceId(123), MosaicId(id), Height(345), key, Amount(456), BlockDuration(567));
				return MosaicDescriptor(pEntry, index, isActive);
			}

			static MosaicDescriptor CreateElement(const state::MosaicEntry& entry, uint32_t index, bool isActive) {
				auto pEntryCopy = std::make_shared<state::MosaicEntry>(entry);
				return MosaicDescriptor(pEntryCopy, index, isActive);
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& descriptor) {
				auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
				mosaicCacheDelta.insert(*descriptor.pEntry);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& descriptor) {
				auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
				mosaicCacheDelta.remove(descriptor.pEntry->mosaicId());
			}

			static MosaicDescriptor Mutate(cache::CatapultCacheDelta& delta, ModelType& descriptor) {
				auto pEntryCopy = std::make_shared<state::MosaicEntry>(*descriptor.pEntry);
				pEntryCopy->increaseSupply(Amount(1));

				// update database and return new descriptor
				auto modifiedDescriptor = MosaicDescriptor(pEntryCopy, descriptor.Index + 1, true);
				Add(delta, modifiedDescriptor);
				return modifiedDescriptor;
			}

			static auto GetFindFilter(const ModelType& descriptor) {
				const auto& entry = *descriptor.pEntry;
				return document()
						<< "$and"
						<< open_array
							<< open_document << "mosaic.namespaceId" << mappers::ToInt64(entry.namespaceId()) << close_document
							<< open_document << "mosaic.mosaicId" << mappers::ToInt64(entry.mosaicId()) << close_document
							<< open_document << "meta.index" << static_cast<int32_t>(descriptor.Index) << close_document
						<< close_array
						<< finalize;
			}

			static void AssertEqual(const ModelType& descriptor, const bsoncxx::document::view& view) {
				test::AssertEqualMosaicData(descriptor, view["mosaic"].get_document().view());
			}
		};
	}

	DEFINE_HISTORICAL_CACHE_STORAGE_TESTS(MosaicCacheTraits,);
}}}
