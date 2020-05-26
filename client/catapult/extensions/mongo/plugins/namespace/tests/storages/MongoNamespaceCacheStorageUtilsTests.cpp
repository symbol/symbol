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

#include "src/storages/MongoNamespaceCacheStorageUtils.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/RootNamespaceHistory.h"
#include "plugins/txes/namespace/tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoNamespaceCacheStorageUtilsTests

	namespace {
		enum class EntityStatus { Active, Inactive };
	}

	// region NamespaceDescriptorsFromHistory

	namespace {
		using Path = state::Namespace::Path;

		struct PathIndexPair {
		public:
			PathIndexPair(const Path& path, uint32_t index)
					: NamespacePath(path)
					, Index(index)
			{}

		public:
			bool operator==(const PathIndexPair& rhs) const {
				return NamespacePath == rhs.NamespacePath && Index == rhs.Index;
			}

		public:
			Path NamespacePath;
			uint32_t Index;
		};

		struct PathIndexPairHasher {
			size_t operator()(const PathIndexPair& pair) const {
				const auto& path = pair.NamespacePath;
				auto hash = static_cast<size_t>(path[0].unwrap() ^ pair.Index);
				for (auto i = 1u; i < path.size(); ++i)
					hash ^= path[i].unwrap();

				return hash;
			}
		};

		struct PathIndexPairEquality {
			bool operator()(const PathIndexPair& lhs, const PathIndexPair& rhs) const {
				return lhs == rhs;
			}
		};

		using DescriptorMap = std::unordered_map<PathIndexPair, NamespaceDescriptor, PathIndexPairHasher, PathIndexPairEquality>;

		DescriptorMap ToDescriptorMap(const std::vector<NamespaceDescriptor>& descriptors) {
			DescriptorMap map;
			for (const auto& descriptor : descriptors)
				map.emplace(PathIndexPair(descriptor.Path, descriptor.Index), descriptor);

			return map;
		}

		void AssertNamespaceDescriptorExists(
				uint32_t rootIndex,
				EntityStatus status,
				const Path& path,
				const state::RootNamespace& root,
				const DescriptorMap& descriptorMap) {
			const auto& descriptor = descriptorMap.at(PathIndexPair(path, rootIndex));
			EXPECT_EQ(path, descriptor.Path);
			EXPECT_EQ(&root, descriptor.pRoot.get());
			EXPECT_EQ(root.ownerAddress(), descriptor.OwnerAddress);
			EXPECT_EQ(rootIndex, descriptor.Index);
			EXPECT_EQ(EntityStatus::Active == status, descriptor.IsActive);

			const auto& alias = root.alias(state::Namespace(path).id());
			test::AssertEqualAlias(alias, descriptor.Alias);
		}

		void AddChildren(state::RootNamespace& root, const std::vector<Path>& paths) {
			for (const auto& path : paths)
				root.add(state::Namespace(path));
		}
	}

	TEST(TEST_CLASS, CanConvertFromEmptyRootNamespaceHistory) {
		// Arrange:
		state::RootNamespaceHistory history(NamespaceId(123));

		// Act:
		auto descriptors = NamespaceDescriptorsFromHistory(history);

		// Assert:
		EXPECT_TRUE(descriptors.empty());
	}

	namespace {
		template<typename TMutate>
		void AssertCanConvertFromRootNamespaceHistoryWithDepthOne_WithoutChildren(TMutate mutate) {
			// Arrange:
			state::RootNamespaceHistory history(NamespaceId(123));
			history.push_back(test::CreateRandomOwner(), test::CreateLifetime(234, 345));

			auto& root = history.back();
			mutate(root);

			// Act:
			auto descriptors = NamespaceDescriptorsFromHistory(history);
			auto descriptorMap = ToDescriptorMap(descriptors);

			// Assert:
			EXPECT_EQ(1u, descriptors.size());

			AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123 }), root, descriptorMap);
		}
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthOne_WithoutChildren) {
		AssertCanConvertFromRootNamespaceHistoryWithDepthOne_WithoutChildren([](const auto&) {});
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthOne_WithoutChildren_WithAlias) {
		AssertCanConvertFromRootNamespaceHistoryWithDepthOne_WithoutChildren([](auto& root) {
			// Arrange: set a root alias
			root.setAlias(NamespaceId(123), state::NamespaceAlias(MosaicId(444)));
		});
	}

	namespace {
		template<typename TMutate>
		void AssertCanConvertFromRootNamespaceHistoryWithDepthOne_WithChildren(TMutate mutate) {
			// Arrange:
			state::RootNamespaceHistory history(NamespaceId(123));
			history.push_back(test::CreateRandomOwner(), test::CreateLifetime(234, 345));

			auto& root = history.back();
			AddChildren(root, { test::CreatePath({ 123, 124 }), test::CreatePath({ 123, 124, 126 }) });
			mutate(root);

			// Act:
			auto descriptors = NamespaceDescriptorsFromHistory(history);
			auto descriptorMap = ToDescriptorMap(descriptors);

			// Assert:
			EXPECT_EQ(3u, descriptors.size());

			AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123 }), root, descriptorMap);
			AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123, 124 }), root, descriptorMap);
			AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123, 124, 126 }), root, descriptorMap);
		}
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthOne_WithChildren) {
		AssertCanConvertFromRootNamespaceHistoryWithDepthOne_WithChildren([](const auto&) {});
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthOne_WithChildren_WithAlias) {
		AssertCanConvertFromRootNamespaceHistoryWithDepthOne_WithChildren([](auto& root) {
			// Arrange: set child aliases
			root.setAlias(NamespaceId(124), state::NamespaceAlias(MosaicId(444)));
			root.setAlias(NamespaceId(126), state::NamespaceAlias(test::GenerateRandomByteArray<Address>()));
		});
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthGreaterThanOne_WithoutChildren) {
		// Arrange:
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(test::CreateRandomOwner(), test::CreateLifetime(234, 345));
		history.push_back(test::CreateRandomOwner(), test::CreateLifetime(456, 567));
		history.push_back(test::CreateRandomOwner(), test::CreateLifetime(678, 789));

		// Act:
		auto descriptors = NamespaceDescriptorsFromHistory(history);
		auto descriptorMap = ToDescriptorMap(descriptors);

		// Assert:
		EXPECT_EQ(3u, descriptors.size());

		const auto& root1 = history.back();
		AssertNamespaceDescriptorExists(2, EntityStatus::Active, test::CreatePath({ 123 }), root1, descriptorMap);
		history.pop_back();
		const auto& root2 = history.back();
		AssertNamespaceDescriptorExists(1, EntityStatus::Inactive, test::CreatePath({ 123 }), root2, descriptorMap);
		history.pop_back();
		const auto& root3 = history.back();
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123 }), root3, descriptorMap);
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthGreaterThanOne_WithChildren) {
		// Arrange:
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(test::CreateRandomOwner(), test::CreateLifetime(234, 345));
		AddChildren(history.back(), { test::CreatePath({ 123, 124 }), test::CreatePath({ 123, 124, 126 }) });
		history.push_back(test::CreateRandomOwner(), test::CreateLifetime(456, 567));
		AddChildren(history.back(), { test::CreatePath({ 123, 130 }), test::CreatePath({ 123, 131 }) });
		history.push_back(test::CreateRandomOwner(), test::CreateLifetime(678, 789));
		AddChildren(history.back(), { test::CreatePath({ 123, 150 }) });

		// Act:
		auto descriptors = NamespaceDescriptorsFromHistory(history);
		auto descriptorMap = ToDescriptorMap(descriptors);

		// Assert:
		EXPECT_EQ(8u, descriptors.size());

		const auto& root1 = history.back();
		AssertNamespaceDescriptorExists(2, EntityStatus::Active, test::CreatePath({ 123, 150 }), root1, descriptorMap);
		AssertNamespaceDescriptorExists(2, EntityStatus::Active, test::CreatePath({ 123 }), root1, descriptorMap);
		history.pop_back();
		const auto& root2 = history.back();
		AssertNamespaceDescriptorExists(1, EntityStatus::Inactive, test::CreatePath({ 123, 131 }), root2, descriptorMap);
		AssertNamespaceDescriptorExists(1, EntityStatus::Inactive, test::CreatePath({ 123, 130 }), root2, descriptorMap);
		AssertNamespaceDescriptorExists(1, EntityStatus::Inactive, test::CreatePath({ 123 }), root2, descriptorMap);
		history.pop_back();
		const auto& root3 = history.back();
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123, 124, 126 }), root3, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123, 124 }), root3, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123 }), root3, descriptorMap);
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthGreaterThanOne_SameOwner_WithChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(234, 345));
		AddChildren(history.back(), { test::CreatePath({ 123, 124 }), test::CreatePath({ 123, 124, 126 }) });
		history.push_back(owner, test::CreateLifetime(456, 567));
		AddChildren(history.back(), { test::CreatePath({ 123, 130 }) });

		// Act:
		auto descriptors = NamespaceDescriptorsFromHistory(history);
		auto descriptorMap = ToDescriptorMap(descriptors);

		// Assert: both roots have the same set of children
		EXPECT_FALSE(descriptors.empty());
		EXPECT_EQ(8u, descriptors.size());

		const auto& root1 = history.back();
		AssertNamespaceDescriptorExists(1, EntityStatus::Active, test::CreatePath({ 123, 130 }), root1, descriptorMap);
		AssertNamespaceDescriptorExists(1, EntityStatus::Active, test::CreatePath({ 123, 124 }), root1, descriptorMap);
		AssertNamespaceDescriptorExists(1, EntityStatus::Active, test::CreatePath({ 123, 124, 126 }), root1, descriptorMap);
		AssertNamespaceDescriptorExists(1, EntityStatus::Active, test::CreatePath({ 123 }), root1, descriptorMap);
		history.pop_back();
		const auto& root2 = history.back();
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123, 130 }), root2, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123, 124 }), root2, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123, 124, 126 }), root2, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Inactive, test::CreatePath({ 123 }), root2, descriptorMap);
		history.pop_back();
	}

	// endregion
}}}
