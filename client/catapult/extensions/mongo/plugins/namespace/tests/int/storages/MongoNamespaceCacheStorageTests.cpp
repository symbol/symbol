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
					<< "$and" << open_array
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
				return CreateElement(test::CreateRandomOwner(), id, index, isActive);
			}

			static NamespaceDescriptor CreateElement(const Address& owner, uint32_t id, uint32_t index, bool isActive) {
				auto alias = GetNamespaceAlias(NamespaceId(id));
				auto pRoot = std::make_shared<state::RootNamespace>(NamespaceId(id), owner, test::CreateLifetime(123, 456));
				return NamespaceDescriptor(CreateRootPath(NamespaceId(id)), alias, pRoot, owner, index, isActive);
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
			auto owner = test::CreateRandomOwner();
			auto pChangedRoot = std::make_shared<state::RootNamespace>(descriptor.pRoot->id(), owner, descriptor.pRoot->lifetime());

			// update cache and return new descriptor
			const auto& path = descriptor.Path;
			auto modifiedDescriptor = NamespaceDescriptor(path, descriptor.Alias, pChangedRoot, owner, descriptor.Index + 1, true);
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
}}}
