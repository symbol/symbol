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

#include "src/cache/NamespaceCacheStorage.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/RootNamespaceHistoryLoadTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS NamespaceCacheStorageTests

	namespace {
		constexpr auto Default_Cache_Options = NamespaceCacheTypes::Options{ BlockDuration(100) };

		void LoadInto(io::InputStream& inputStream, NamespaceCacheDelta& delta) {
			return NamespaceCacheStorage::LoadInto(NamespaceCacheStorage::Load(inputStream), delta);
		}

		state::NamespaceAlias GetAlias(const NamespaceCacheView& view, NamespaceId id) {
			return view.find(id).get().root().alias(id);
		}

		state::NamespaceAlias GetAlias(const NamespaceCacheDelta& delta, NamespaceId id) {
			return delta.find(id).get().root().alias(id);
		}

		struct LoadTraits {
			using NamespaceHistoryHeader = test::NamespaceHistoryHeader;

			static NamespaceHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, uint64_t depth) {
				return { depth, namespaceId };
			}

			template<typename TException>
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				NamespaceCache cache(CacheConfiguration{}, Default_Cache_Options);
				auto delta = cache.createDelta();
				EXPECT_THROW(LoadInto(inputStream, *delta), TException);
			}

			static void AssertCanLoadHistoryWithDepthOneWithoutChildren(
					io::InputStream& inputStream,
					const Key& owner,
					const state::NamespaceAlias& alias) {
				// Act:
				NamespaceCache cache(CacheConfiguration{}, Default_Cache_Options);
				auto delta = cache.createDelta();
				LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				test::AssertCacheSizes(*view, 1, 1, 1);

				ASSERT_TRUE(view->contains(NamespaceId(123)));
				test::AssertRootNamespace(view->find(NamespaceId(123)).get().root(), owner, Height(222), Height(333), 0, alias);
			}

			static void AssertCannotLoadHistoryWithDepthOneOutOfOrderChildren(io::InputStream& inputStream) {
				// Arrange:
				NamespaceCache cache(CacheConfiguration{}, Default_Cache_Options);
				auto delta = cache.createDelta();

				// Act + Assert:
				EXPECT_THROW(LoadInto(inputStream, *delta), catapult_invalid_argument);
			}

			static void AssertCanLoadHistoryWithDepthOneWithChildren(
					io::InputStream& inputStream,
					const Key& owner,
					const std::vector<state::NamespaceAlias>& aliases) {
				// Act:
				NamespaceCache cache(CacheConfiguration{}, Default_Cache_Options);
				auto delta = cache.createDelta();
				LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				test::AssertCacheSizes(*view, 1, 4, 4);

				ASSERT_TRUE(view->contains(NamespaceId(123)));
				test::AssertRootNamespace(view->find(NamespaceId(123)).get().root(), owner, Height(222), Height(333), 3);
				EXPECT_EQ(NamespaceId(123), view->find(NamespaceId(124)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(124), view->find(NamespaceId(125)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(123), view->find(NamespaceId(126)).get().ns().parentId());

				// - check aliases
				ASSERT_EQ(3u, aliases.size());
				test::AssertEqualAlias(aliases[0], GetAlias(*view, NamespaceId(124)), "124");
				test::AssertEqualAlias(aliases[1], GetAlias(*view, NamespaceId(125)), "125");
				test::AssertEqualAlias(aliases[2], GetAlias(*view, NamespaceId(126)), "126");
			}

			static void AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner(
					io::InputStream& inputStream,
					const Key& owner,
					const std::vector<state::NamespaceAlias>& aliases) {
				// Act:
				NamespaceCache cache(CacheConfiguration{}, Default_Cache_Options);
				auto delta = cache.createDelta();
				LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				test::AssertCacheSizes(*view, 1, 5, 15);

				ASSERT_TRUE(view->contains(NamespaceId(123)));
				test::AssertRootNamespace(view->find(NamespaceId(123)).get().root(), owner, Height(444), Height(555), 4);
				EXPECT_EQ(NamespaceId(123), view->find(NamespaceId(124)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(124), view->find(NamespaceId(125)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(123), view->find(NamespaceId(126)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(126), view->find(NamespaceId(129)).get().ns().parentId());

				// - check aliases
				ASSERT_EQ(4u, aliases.size());
				test::AssertEqualAlias(aliases[0], GetAlias(*view, NamespaceId(124)), "124");
				test::AssertEqualAlias(aliases[1], GetAlias(*view, NamespaceId(125)), "125");
				test::AssertEqualAlias(aliases[2], GetAlias(*view, NamespaceId(126)), "126");
				test::AssertEqualAlias(aliases[3], GetAlias(*view, NamespaceId(129)), "129");

				// - check history (one back)
				delta->remove(NamespaceId(123));
				test::AssertCacheSizes(*delta, 1, 5, 10);
				test::AssertRootNamespace(delta->find(NamespaceId(123)).get().root(), owner, Height(222), Height(333), 4);

				// - check history (two back)
				delta->remove(NamespaceId(123));
				test::AssertCacheSizes(*delta, 1, 5, 5);
				test::AssertRootNamespace(delta->find(NamespaceId(123)).get().root(), owner, Height(11), Height(111), 4);
			}

			static void AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner(
					io::InputStream& inputStream,
					const Key& owner1,
					const Key& owner2,
					const Key& owner3,
					const std::vector<state::NamespaceAlias>& aliases) {
				// Act:
				NamespaceCache cache(CacheConfiguration{}, Default_Cache_Options);
				auto delta = cache.createDelta();
				LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				test::AssertCacheSizes(*view, 1, 2, 7);

				ASSERT_TRUE(view->contains(NamespaceId(123)));
				test::AssertRootNamespace(view->find(NamespaceId(123)).get().root(), owner3, Height(444), Height(555), 1);
				EXPECT_EQ(NamespaceId(123), view->find(NamespaceId(126)).get().ns().parentId());

				// - check aliases
				ASSERT_EQ(4u, aliases.size());
				test::AssertEqualAlias(aliases[3], GetAlias(*view, NamespaceId(126)), "126 :: 0");

				// - check history (one back)
				delta->remove(NamespaceId(123));
				test::AssertCacheSizes(*delta, 1, 4, 5);
				test::AssertRootNamespace(delta->find(NamespaceId(123)).get().root(), owner2, Height(222), Height(333), 3);
				EXPECT_EQ(NamespaceId(123), delta->find(NamespaceId(124)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(124), delta->find(NamespaceId(125)).get().ns().parentId());
				EXPECT_EQ(NamespaceId(123), delta->find(NamespaceId(126)).get().ns().parentId());

				// - check aliases
				test::AssertEqualAlias(aliases[0], GetAlias(*delta, NamespaceId(124)), "124");
				test::AssertEqualAlias(aliases[1], GetAlias(*delta, NamespaceId(125)), "125");
				test::AssertEqualAlias(aliases[2], GetAlias(*delta, NamespaceId(126)), "126 :: 1");

				// - check history (two back)
				delta->remove(NamespaceId(123));
				test::AssertCacheSizes(*delta, 1, 1, 1);
				test::AssertRootNamespace(delta->find(NamespaceId(123)).get().root(), owner1, Height(11), Height(111), 0);
			}
		};
	}

	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_TESTS(LoadTraits,)
}}
