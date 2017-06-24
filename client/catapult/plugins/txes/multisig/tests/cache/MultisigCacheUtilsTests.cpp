#include "src/cache/MultisigCacheUtils.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"

#define TEST_CLASS MultisigCacheUtilsTests

namespace catapult { namespace cache {

	namespace {
		constexpr auto Num_Tree_Accounts = 14;

		auto CreateCacheMultisigTree(const std::vector<Key>& keys) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			// D   2 \   / 7 - A - B |
			//   \ 3 - 6 - 8     \   |
			// 1 - 4 /   \ 9       C |
			//   \                   |
			// 0 - 5                 |
			test::MakeMultisig(cacheDelta, keys[13], { keys[4] });
			test::MakeMultisig(cacheDelta, keys[1], { keys[4], keys[5] });
			test::MakeMultisig(cacheDelta, keys[0], { keys[5] });

			test::MakeMultisig(cacheDelta, keys[2], { keys[6] });
			test::MakeMultisig(cacheDelta, keys[3], { keys[6] });
			test::MakeMultisig(cacheDelta, keys[4], { keys[6] });

			test::MakeMultisig(cacheDelta, keys[6], { keys[7], keys[8], keys[9] });
			test::MakeMultisig(cacheDelta, keys[7], { keys[10] });
			test::MakeMultisig(cacheDelta, keys[10], { keys[11], keys[12] });

			cache.commit(Height());
			return cache;
		}

		template<typename TAction>
		void RunMultisigTreeTest(TAction action) {
			// Arrange: generate random keys but change the second byte of each to its index (for easier diagnosing of failures)
			auto keys = test::GenerateKeys(Num_Tree_Accounts);
			uint8_t id = 0;
			for (auto& key : keys)
				key[1] = id++;

			auto cache = CreateCacheMultisigTree(keys);
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();

			// Act:
			action(readOnlyCache.sub<cache::MultisigCache>(), keys);
		}
	}

	// region FindDescendants

	TEST(TEST_CLASS, LeafNodesDoNotHaveDescendants) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& keys) {
			// Act:
			utils::KeySet descendantKeys;
			auto numLevels = FindDescendants(cache, keys[11], descendantKeys);

			// Assert:
			EXPECT_EQ(0u, numLevels);
			EXPECT_TRUE(descendantKeys.empty());
		});
	}

	TEST(TEST_CLASS, CanFindDirectDescendants) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& keys) {
			// Act:
			utils::KeySet descendantKeys;
			auto numLevels = FindDescendants(cache, keys[10], descendantKeys);

			// Assert:
			EXPECT_EQ(1u, numLevels);
			EXPECT_EQ(utils::KeySet({ keys[11], keys[12] }), descendantKeys);
		});
	}

	TEST(TEST_CLASS, CanFindAllDescendants) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& keys) {
			// Act:
			utils::KeySet descendantKeys;
			auto numLevels = FindDescendants(cache, keys[4], descendantKeys);

			// Assert:
			EXPECT_EQ(4u, numLevels);
			EXPECT_EQ(utils::KeySet({ keys[6], keys[7], keys[8], keys[9], keys[10], keys[11], keys[12] }), descendantKeys);
		});
	}

	// endregion

	// region FindAncestors

	TEST(TEST_CLASS, RootNodesDoNotHaveAncestors) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& keys) {
			// Act:
			utils::KeySet ancestorKeys;
			auto numLevels = FindAncestors(cache, keys[1], ancestorKeys);

			// Assert:
			EXPECT_EQ(0u, numLevels);
			EXPECT_TRUE(ancestorKeys.empty());
		});
	}

	TEST(TEST_CLASS, CanFindDirectAncestors) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& keys) {
			// Act:
			utils::KeySet ancestorKeys;
			auto numLevels = FindAncestors(cache, keys[4], ancestorKeys);

			// Assert:
			EXPECT_EQ(1u, numLevels);
			EXPECT_EQ(utils::KeySet({ keys[1], keys[13] }), ancestorKeys);
		});
	}

	TEST(TEST_CLASS, CanFindAllAncestors) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& keys) {
			// Act:
			utils::KeySet ancestorKeys;
			auto numLevels = FindAncestors(cache, keys[10], ancestorKeys);

			// Assert:
			EXPECT_EQ(4u, numLevels);
			EXPECT_EQ(utils::KeySet({ keys[7], keys[6], keys[2], keys[3], keys[4], keys[1], keys[13] }), ancestorKeys);
		});
	}

	// endregion
}}
