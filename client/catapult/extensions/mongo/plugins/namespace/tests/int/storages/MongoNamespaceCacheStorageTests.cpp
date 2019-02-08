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

#include "src/storages/MongoNamespaceCacheStorage.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Address.h"
#include "mongo/tests/test/MongoHistoricalCacheStorageTests.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceCacheTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceTestUtils.h"
#include "tests/test/NamespaceMapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoNamespaceCacheStorageTests

	namespace {
		using Path = state::Namespace::Path;

		bsoncxx::document::value CreateNamespacePathFilter(const Path& path, uint32_t index) {
			document doc;
			auto filter = doc
					<< "$and"
					<< open_array
					<< open_document << "namespace.level0" << mappers::ToInt64(path[0]) << close_document;

			if (1 < path.size())
				filter << open_document << "namespace.level1" << mappers::ToInt64(path[1]) << close_document;

			if (2 < path.size())
				filter << open_document << "namespace.level2" << mappers::ToInt64(path[2]) << close_document;

			filter
					<< open_document << "meta.index" << static_cast<int32_t>(index) << close_document
					<< open_document << "namespace.depth" << static_cast<int32_t>(path.size()) << close_document
					<< close_array;

			return doc << finalize;
		}

		Path CreateRootPath(NamespaceId id) {
			Path path;
			path.push_back(id);
			return path;
		}

		state::NamespaceAlias GetNamespaceAlias(NamespaceId id) {
			// generate all alias types deterministically based on the id
			switch (id.unwrap() % 3) {
			case 1:
				return state::NamespaceAlias(MosaicId(3 * id.unwrap()));

			case 2:
				return state::NamespaceAlias(Address{ { static_cast<uint8_t>(2 * id.unwrap()) } });

			default:
				return state::NamespaceAlias();
			}
		}

		state::NamespaceAlias GetNamespaceAlias(NamespaceId id, size_t adjustment) {
			return GetNamespaceAlias(NamespaceId(id.unwrap() + adjustment));
		}

		struct NamespaceCacheTraits {
			using CacheType = cache::NamespaceCache;
			using ModelType = NamespaceDescriptor;

			static constexpr auto Collection_Name = "namespaces";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoNamespaceCacheStorage;

			static cache::CatapultCache CreateCache() {
				return test::NamespaceCacheFactory::Create();
			}

			static NamespaceDescriptor GenerateRandomElement(uint32_t id, uint32_t index, bool isActive) {
				return CreateElement(test::GenerateRandomData<Key_Size>(), id, index, isActive);
			}

			static NamespaceDescriptor CreateElement(const Key& key, uint32_t id, uint32_t index, bool isActive) {
				auto alias = GetNamespaceAlias(NamespaceId(id));
				auto pRoot = std::make_shared<state::RootNamespace>(NamespaceId(id), key, test::CreateLifetime(123, 456));
				auto address = model::PublicKeyToAddress(key, Network_Id);
				return NamespaceDescriptor(CreateRootPath(NamespaceId(id)), alias, pRoot, address, index, isActive);
			}

			static NamespaceDescriptor CreateElement(const state::RootNamespace& root, uint32_t index, bool isActive) {
				auto alias = GetNamespaceAlias(NamespaceId(root.id()));
				auto pRootCopy = std::make_shared<state::RootNamespace>(root);
				auto address = model::PublicKeyToAddress(root.owner(), Network_Id);
				return NamespaceDescriptor(CreateRootPath(pRootCopy->id()), alias, pRootCopy, address, index, isActive);
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& descriptor) {
				auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
				namespaceCacheDelta.insert(*descriptor.pRoot);
				namespaceCacheDelta.setAlias(descriptor.pRoot->id(), descriptor.Alias);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& descriptor) {
				auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
				namespaceCacheDelta.remove(descriptor.pRoot->id());
			}

			static auto GetFindFilter(const ModelType& descriptor) {
				return CreateNamespacePathFilter(descriptor.Path, descriptor.Index);
			}

			static void AssertEqual(const ModelType& descriptor, const bsoncxx::document::view& view) {
				test::AssertEqualNamespaceData(descriptor, view["namespace"].get_document().view());
			}
		};
	}

	struct NamespaceCacheRootModificationTraits : public NamespaceCacheTraits {
		static NamespaceDescriptor Mutate(cache::CatapultCacheDelta& delta, ModelType& descriptor) {
			// change owner
			auto key = test::GenerateRandomData<Key_Size>();
			auto pChangedRoot = std::make_shared<state::RootNamespace>(descriptor.pRoot->id(), key, descriptor.pRoot->lifetime());
			auto address = model::PublicKeyToAddress(key, Network_Id);

			// update cache and return new descriptor
			const auto& path = descriptor.Path;
			auto modifiedDescriptor = NamespaceDescriptor(path, descriptor.Alias, pChangedRoot, address, descriptor.Index + 1, true);
			Add(delta, modifiedDescriptor);
			return modifiedDescriptor;
		}
	};

	struct NamespaceCacheChildModificationTraits : public NamespaceCacheTraits {
		static NamespaceDescriptor Mutate(cache::CatapultCacheDelta& delta, ModelType& descriptor) {
			// insert a child
			auto childPath = descriptor.Path;
			childPath.push_back(NamespaceId(987));

			// update cache and return new descriptor
			auto& namespaceCacheDelta = delta.sub<cache::NamespaceCache>();
			auto childNamespace = state::Namespace(childPath);
			namespaceCacheDelta.insert(childNamespace);
			namespaceCacheDelta.setAlias(childNamespace.id(), descriptor.Alias);
			return NamespaceDescriptor(childPath, descriptor.Alias, descriptor.pRoot, descriptor.OwnerAddress, descriptor.Index, true);
		}
	};

	// modifications that create historical entries (increasing indexes)
	DEFINE_HISTORICAL_CACHE_STORAGE_TESTS(NamespaceCacheRootModificationTraits, _RootModification)

	// modifications that create children (path filter is passed path with multiple parts)
	DEFINE_HISTORICAL_CACHE_STORAGE_TESTS(NamespaceCacheChildModificationTraits, _ChildModification)

	// region custom load tests - load all with children

	namespace {
		struct RootNamespaceDescriptor {
		private:
			using ChildrenPaths = std::vector<std::vector<NamespaceId::ValueType>>;

		public:
			// note that the constructor is not explicit to allow an initializer list for the children
			RootNamespaceDescriptor(NamespaceId::ValueType id, const Key& owner, const ChildrenPaths& paths)
					: Id(id)
					, Owner(owner)
					, Children(paths)
				{}

		public:
			NamespaceId::ValueType Id;
			Key Owner;
			ChildrenPaths Children;
		};

		// increment must be zero for all tests relying on element equals behavior
		// use nonzero increment to have aliases change across levels
		void SeedCache(
				const std::vector<RootNamespaceDescriptor>& seeds,
				cache::NamespaceCacheDelta& namespaceCacheDelta,
				size_t increment = 0) {
			size_t adjustment = 0;
			auto lifetime = test::CreateLifetime(123, 456);
			for (const auto& rootDescriptor : seeds) {
				auto pRoot = std::make_shared<state::RootNamespace>(NamespaceId(rootDescriptor.Id), rootDescriptor.Owner, lifetime);
				namespaceCacheDelta.insert(*pRoot);
				namespaceCacheDelta.setAlias(pRoot->id(), GetNamespaceAlias(pRoot->id(), adjustment));

				for (const auto& childPath : rootDescriptor.Children) {
					auto childNamespace = state::Namespace(test::CreatePath(childPath));
					namespaceCacheDelta.insert(childNamespace);
					namespaceCacheDelta.setAlias(childNamespace.id(), GetNamespaceAlias(childNamespace.id(), adjustment));
				}

				adjustment += increment;
			}
		}

		void AssertCanLoadAll(const std::vector<RootNamespaceDescriptor>& seeds, size_t expectedCacheSize, size_t expectedDeepSize) {
			// Arrange:
			using Runner = test::MongoHistoricalCacheStorageTests<NamespaceCacheRootModificationTraits>;
			Runner::RunCustomLoadTest([&seeds, expectedCacheSize, expectedDeepSize](auto& delta) {
				auto& namespaceCacheDelta = delta.template sub<cache::NamespaceCache>();
				SeedCache(seeds, namespaceCacheDelta);
				return std::make_pair(expectedCacheSize, expectedDeepSize);
			});
		}
	}

	TEST(TEST_CLASS, CanLoadAll_SameOwner_WithChildren) {
		// Arrange: id / owner / children
		// - note that this simulates the following sequence:
		//   1) first root is added, then children with ids { 10, 11, 12 } are added
		//   2) second root is added, then child with id { 13 } is added
		//   3) third root is added, then  child with id { 14 } is added
		//   since roots with same owner share children, all roots refer to the same set of children
		auto owner = test::GenerateRandomData<Key_Size>();
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, owner, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } } },
			{ 1, owner, { { 1, 12, 13 } } },
			{ 1, owner, { { 1, 14 } } }
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3, 6 + 6 + 6);
	}

	TEST(TEST_CLASS, CanLoadAll_AlternatingOwner_WithChildren) {
		// Arrange: id / owner / children
		// - note that since the owner changes from root to root, there is no inheritance of children
		auto owner1 = test::GenerateRandomData<Key_Size>();
		auto owner2 = test::GenerateRandomData<Key_Size>();
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } } },
			{ 1, owner2, { { 1, 13 } } },
			{ 1, owner1, { { 1, 15 }, { 1, 15, 16 } } },
		};

		// Assert:
		AssertCanLoadAll(seedEntries, 3, 4 + 2 + 3);
	}

	// endregion

	// region custom load tests - aliases

	namespace {
		std::pair<size_t, size_t> AssertLoadAllPreservesAliases(
				const std::vector<RootNamespaceDescriptor>& seeds,
				const std::vector<size_t>& expectedChildCounts) {
			// Arrange:
			auto seeder = [&seeds](auto& delta) {
				// - seed the cache with a nonzero increment to ensure all aliases are different across all levels
				auto& namespaceCacheDelta = delta.template sub<cache::NamespaceCache>();
				SeedCache(seeds, namespaceCacheDelta, 1);
			};

			// Act:
			using Runner = test::MongoHistoricalCacheStorageTests<NamespaceCacheRootModificationTraits>;
			auto counts = std::make_pair<size_t, size_t>(0, 0);
			Runner::RunCustomLoadTest(seeder, [&seeds, &expectedChildCounts, &counts](auto& cache) {
				auto cacheDelta = cache.createDelta();
				auto& namespaceCacheDelta = cacheDelta.template sub<cache::NamespaceCache>();

				// Assert: iterate over (root) seeds in reverse order (last seed is added last so it is at the top of the history)
				auto i = 0u;
				for (auto iter = seeds.crbegin(); seeds.crend() != iter; ++iter) {
					const auto& rootDescriptor = *iter;
					auto rootId = NamespaceId(rootDescriptor.Id);
					const auto& root = namespaceCacheDelta.find(rootId).get().root();

					// - check the root alias and the number of children
					auto adjustment = seeds.size() - 1 - i;
					std::ostringstream rootMessage;
					rootMessage << "root " << rootId << " (level " << i << ")";
					test::AssertEqualAlias(GetNamespaceAlias(rootId, adjustment), root.alias(rootId), rootMessage.str());
					EXPECT_EQ(expectedChildCounts[i], root.size()) << rootMessage.str();

					// - iterate over (child) seeds in reverse order (last child is added last and could depend on prior children)
					for (auto childIter = rootDescriptor.Children.crbegin(); rootDescriptor.Children.crend() != childIter; ++childIter) {
						const auto& childPath = *childIter;
						auto childId = NamespaceId(childPath[childPath.size() - 1]);

						// - check the child alias
						std::ostringstream childMessage;
						childMessage << "child " << childId << " (level " << i << ")";
						test::AssertEqualAlias(GetNamespaceAlias(childId, adjustment), root.alias(childId), childMessage.str());

						// - remove the child (this allows root to be popped below)
						namespaceCacheDelta.remove(childId);
						++counts.second;
					}

					// - pop the root (to activate previous root)
					namespaceCacheDelta.remove(rootId);
					++counts.first;
					++i;
				}

				// Sanity: the root should have been completely removed (this test assumes all root descriptors have the same root id)
				EXPECT_FALSE(namespaceCacheDelta.contains(NamespaceId(seeds[0].Id)));
			});

			return counts;
		}
	}

	TEST(TEST_CLASS, LoadAllPreservesAliases_SameOwner_WithChildren) {
		// Arrange: see CanLoadAll_SameOwner_WithChildren
		auto owner = test::GenerateRandomData<Key_Size>();
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, owner, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } } },
			{ 1, owner, { { 1, 12, 13 } } },
			{ 1, owner, { { 1, 14 } } }
		};

		// Assert:
		auto counts = AssertLoadAllPreservesAliases(seedEntries, { 5, 4, 3 });
		EXPECT_EQ(3u, counts.first);
		EXPECT_EQ(3u + 1 + 1, counts.second);
	}

	TEST(TEST_CLASS, LoadAllPreservesAliases_AlternatingOwner_WithChildren) {
		// Arrange: see CanLoadAll_AlternatingOwner_WithChildren
		auto owner1 = test::GenerateRandomData<Key_Size>();
		auto owner2 = test::GenerateRandomData<Key_Size>();
		std::vector<RootNamespaceDescriptor> seedEntries{
			{ 1, owner1, { { 1, 10 }, { 1, 10, 11 }, { 1, 12 } } },
			{ 1, owner2, { { 1, 13 } } },
			{ 1, owner1, { { 1, 15 }, { 1, 15, 16 } } },
		};

		// Assert:
		auto counts = AssertLoadAllPreservesAliases(seedEntries, { 2, 1, 3 });
		EXPECT_EQ(3u, counts.first);
		EXPECT_EQ(3u + 1 + 2, counts.second);
	}

	// endregion
}}}
