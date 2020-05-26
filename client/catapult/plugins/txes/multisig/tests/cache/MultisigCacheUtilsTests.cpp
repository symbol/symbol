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

#include "src/cache/MultisigCacheUtils.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "tests/test/MultisigCacheTestUtils.h"
#include "tests/test/MultisigTestUtils.h"

namespace catapult { namespace cache {

#define TEST_CLASS MultisigCacheUtilsTests

	namespace {
		constexpr auto Num_Tree_Accounts = 14;

		auto CreateCacheMultisigTree(const std::vector<Address>& addresses) {
			auto cache = test::MultisigCacheFactory::Create();
			auto cacheDelta = cache.createDelta();

			// D   2 \   / 7 - A - B |
			//   \ 3 - 6 - 8     \   |
			// 1 - 4 /   \ 9       C |
			//   \                   |
			// 0 - 5                 |
			test::MakeMultisig(cacheDelta, addresses[13], { addresses[4] });
			test::MakeMultisig(cacheDelta, addresses[1], { addresses[4], addresses[5] });
			test::MakeMultisig(cacheDelta, addresses[0], { addresses[5] });

			test::MakeMultisig(cacheDelta, addresses[2], { addresses[6] });
			test::MakeMultisig(cacheDelta, addresses[3], { addresses[6] });
			test::MakeMultisig(cacheDelta, addresses[4], { addresses[6] });

			test::MakeMultisig(cacheDelta, addresses[6], { addresses[7], addresses[8], addresses[9] });
			test::MakeMultisig(cacheDelta, addresses[7], { addresses[10] });
			test::MakeMultisig(cacheDelta, addresses[10], { addresses[11], addresses[12] });

			cache.commit(Height());
			return cache;
		}

		template<typename TAction>
		void RunMultisigTreeTest(TAction action) {
			// Arrange: generate random addresses but change the second byte of each to its index (for easier diagnosing of failures)
			auto addresses = test::GenerateRandomDataVector<Address>(Num_Tree_Accounts);
			uint8_t id = 0;
			for (auto& address : addresses)
				address[1] = id++;

			auto cache = CreateCacheMultisigTree(addresses);
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();

			// Act:
			action(readOnlyCache.sub<MultisigCache>(), addresses);
		}

		model::AddressSet Pick(const std::vector<Address>& addresses, std::initializer_list<size_t> indexes) {
			model::AddressSet pickedAddresses;
			for (auto index : indexes)
				pickedAddresses.emplace(addresses[index]);

			return pickedAddresses;
		}
	}

	// region FindDescendants

	TEST(TEST_CLASS, LeafNodesDoNotHaveDescendants) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& addresses) {
			// Act:
			model::AddressSet descendants;
			auto numLevels = FindDescendants(cache, addresses[11], descendants);

			// Assert:
			EXPECT_EQ(0u, numLevels);
			EXPECT_TRUE(descendants.empty());
		});
	}

	TEST(TEST_CLASS, CanFindDirectDescendants) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& addresses) {
			// Act:
			model::AddressSet descendants;
			auto numLevels = FindDescendants(cache, addresses[10], descendants);

			// Assert:
			EXPECT_EQ(1u, numLevels);
			EXPECT_EQ(Pick(addresses, { 11, 12 }), descendants);
		});
	}

	TEST(TEST_CLASS, CanFindAllDescendants) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& addresses) {
			// Act:
			model::AddressSet descendants;
			auto numLevels = FindDescendants(cache, addresses[4], descendants);

			// Assert:
			EXPECT_EQ(4u, numLevels);
			EXPECT_EQ(Pick(addresses, { 6, 7, 8, 9, 10, 11, 12 }), descendants);
		});
	}

	// endregion

	// region FindAncestors

	TEST(TEST_CLASS, RootNodesDoNotHaveAncestors) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& addresses) {
			// Act:
			model::AddressSet ancestors;
			auto numLevels = FindAncestors(cache, addresses[1], ancestors);

			// Assert:
			EXPECT_EQ(0u, numLevels);
			EXPECT_TRUE(ancestors.empty());
		});
	}

	TEST(TEST_CLASS, CanFindDirectAncestors) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& addresses) {
			// Act:
			model::AddressSet ancestors;
			auto numLevels = FindAncestors(cache, addresses[4], ancestors);

			// Assert:
			EXPECT_EQ(1u, numLevels);
			EXPECT_EQ(Pick(addresses, { 1, 13 }), ancestors);
		});
	}

	TEST(TEST_CLASS, CanFindAllAncestors) {
		// Arrange:
		RunMultisigTreeTest([](const auto& cache, const auto& addresses) {
			// Act:
			model::AddressSet ancestors;
			auto numLevels = FindAncestors(cache, addresses[10], ancestors);

			// Assert:
			EXPECT_EQ(4u, numLevels);
			EXPECT_EQ(Pick(addresses, { 7, 6, 2, 3, 4, 1, 13 }), ancestors);
		});
	}

	// endregion
}}
