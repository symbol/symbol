#include "src/storages/CacheStorageUtils.h"
#include "plugins/txes/namespace/src/state/MosaicDescriptor.h"
#include "plugins/txes/namespace/src/state/MosaicHistory.h"
#include "plugins/txes/namespace/src/state/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/RootNamespaceHistory.h"
#include "plugins/txes/namespace/tests/test/MosaicTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS CacheStorageUtilsTests

namespace catapult { namespace mongo { namespace storages {

	namespace {
		enum class EntityStatus { Active, Inactive };

		void AssertMosaicDescriptor(
				const state::MosaicDescriptor& descriptor,
				const state::MosaicEntry& entry,
				uint32_t index,
				EntityStatus status) {
			EXPECT_EQ(&entry, descriptor.pEntry.get());
			EXPECT_EQ(index, descriptor.Index);
			EXPECT_EQ(EntityStatus::Active == status, descriptor.IsActive);
		}
	}

	// region MosaicDescriptorsFromHistory

	TEST(TEST_CLASS, CanConvertFromEmptyMosaicHistory) {
		// Arrange:
		state::MosaicHistory history(NamespaceId(123), MosaicId(234));

		// Act:
		auto descriptors = MosaicDescriptorsFromHistory(history);

		// Assert:
		EXPECT_TRUE(descriptors.empty());
	}

	TEST(TEST_CLASS, CanConvertFromMosaicHistoryWithDepthOne) {
		// Arrange:
		state::MosaicHistory history(NamespaceId(123), MosaicId(234));
		auto original = test::CreateMosaicEntry(MosaicId(234), Amount(345));
		history.push_back(original.definition(), original.supply());
		const auto& entry = history.back();

		// Act:
		auto descriptors = MosaicDescriptorsFromHistory(history);

		// Assert:
		EXPECT_EQ(1u, descriptors.size());

		AssertMosaicDescriptor(descriptors[0], entry, 0, EntityStatus::Active);
	}

	TEST(TEST_CLASS, CanConvertFromMosaicHistoryWithDepthGreaterThanOne) {
		// Arrange:
		state::MosaicHistory history(NamespaceId(123), MosaicId(234));
		auto original1 = test::CreateMosaicEntry(MosaicId(234), Amount(345));
		auto original2 = test::CreateMosaicEntry(MosaicId(234), Amount(543));
		auto original3 = test::CreateMosaicEntry(MosaicId(234), Amount(453));
		history.push_back(original1.definition(), original1.supply());
		history.push_back(original2.definition(), original2.supply());
		history.push_back(original3.definition(), original3.supply());

		// Act:
		auto descriptors = MosaicDescriptorsFromHistory(history);

		// Assert:
		EXPECT_EQ(3u, descriptors.size());

		AssertMosaicDescriptor(descriptors[2], history.back(), 2, EntityStatus::Active);
		history.pop_back();
		AssertMosaicDescriptor(descriptors[1], history.back(), 1, EntityStatus::Inactive);
		history.pop_back();
		AssertMosaicDescriptor(descriptors[0], history.back(), 0, EntityStatus::Inactive);
	}

	// endregion

	// region NamespaceDescriptorsFromHistory

	namespace {
		using Path = state::Namespace::Path;

		struct PathIndexPair {
		public:
			explicit PathIndexPair(const Path& path, uint32_t index)
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
				return static_cast<size_t>(path[0].unwrap() ^
					(1 < path.size() ? path[1].unwrap() : 0) ^
					(2 < path.size() ? path[2].unwrap() : 0) ^
					pair.Index);
			}
		};

		struct PathIndexPairEquality {
			bool operator()(const PathIndexPair& lhs, const PathIndexPair& rhs) const {
				return lhs == rhs;
			}
		};

		using DescriptorMap = std::unordered_map<PathIndexPair, state::NamespaceDescriptor, PathIndexPairHasher, PathIndexPairEquality>;

		DescriptorMap ToDescriptorMap(const std::vector<state::NamespaceDescriptor>& descriptors) {
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
			EXPECT_EQ(rootIndex, descriptor.Index);
			EXPECT_EQ(EntityStatus::Active == status, descriptor.IsActive);
			EXPECT_EQ(path, descriptor.Path);
			EXPECT_EQ(&root, descriptor.pRoot.get());
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

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthOne_WithoutChildren) {
		// Arrange:
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(234, 345));
		const auto& root = history.back();

		// Act:
		auto descriptors = NamespaceDescriptorsFromHistory(history);
		auto descriptorMap = ToDescriptorMap(descriptors);

		// Assert:
		EXPECT_EQ(1u, descriptors.size());

		AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123 }), root, descriptorMap);
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthOne_WithChildren) {
		// Arrange:
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(234, 345));
		auto& root = history.back();
		AddChildren(root, { test::CreatePath({ 123, 124 }), test::CreatePath({ 123, 124, 126 }) });

		// Act:
		auto descriptors = NamespaceDescriptorsFromHistory(history);
		auto descriptorMap = ToDescriptorMap(descriptors);

		// Assert:
		EXPECT_EQ(3u, descriptors.size());

		AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123 }), root, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123, 124 }), root, descriptorMap);
		AssertNamespaceDescriptorExists(0, EntityStatus::Active, test::CreatePath({ 123, 124, 126 }), root, descriptorMap);
	}

	TEST(TEST_CLASS, CanConvertFromRootNamespaceHistoryWithDepthGreaterThanOne_WithoutChildren) {
		// Arrange:
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(234, 345));
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(456, 567));
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(678, 789));

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
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(234, 345));
		AddChildren(history.back(), { test::CreatePath({ 123, 124 }), test::CreatePath({ 123, 124, 126 }) });
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(456, 567));
		AddChildren(history.back(), { test::CreatePath({ 123, 130 }), test::CreatePath({ 123, 131 }) });
		history.push_back(test::GenerateRandomData<Key_Size>(), test::CreateLifetime(678, 789));
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
		auto owner = test::GenerateRandomData<Key_Size>();
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
