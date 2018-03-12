#include "src/storages/MongoMultisigCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Address.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/multisig/tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoMultisigCacheStorageTests

	namespace {
		void InsertRandom(utils::KeySet& keys, size_t count) {
			for (auto i = 0u; i < count; ++i)
				keys.insert(test::GenerateRandomData<Key_Size>());
		}

		struct MultisigCacheTraits {
			using CacheType = cache::MultisigCache;
			using ModelType = state::MultisigEntry;

			static constexpr auto Collection_Name = "multisigs";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoMultisigCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::MultisigCacheFactory::Create();
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				auto key = Key();
				std::memcpy(key.data(), &id, sizeof(id));

				state::MultisigEntry entry(key);
				entry.setMinApproval(12);
				entry.setMinRemoval(23);

				InsertRandom(entry.cosignatories(), std::max<size_t>(1, test::RandomByte() & 0x0F));
				InsertRandom(entry.multisigAccounts(), std::max<size_t>(1, test::RandomByte() & 0x0F));
				return entry;
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& entry) {
				auto& multisigCacheDelta = delta.sub<cache::MultisigCache>();
				multisigCacheDelta.insert(entry);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& entry) {
				auto& multisigCacheDelta = delta.sub<cache::MultisigCache>();
				multisigCacheDelta.remove(entry.key());
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& entry) {
				// update expected
				entry.setMinApproval(24);

				// update cache
				auto& multisigCacheDelta = delta.sub<cache::MultisigCache>();
				auto& entryFromCache = multisigCacheDelta.get(entry.key());
				entryFromCache.setMinApproval(24);
			}

			static auto GetFindFilter(const ModelType& entry) {
				return document() << "multisig.account" << mappers::ToBinary(entry.key()) << finalize;
			}

			static void AssertEqual(const ModelType& entry, const bsoncxx::document::view& view) {
				auto address = model::PublicKeyToAddress(entry.key(), Network_Id);
				test::AssertEqualMultisigData(entry, address, view["multisig"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(MultisigCacheTraits,);
}}}
