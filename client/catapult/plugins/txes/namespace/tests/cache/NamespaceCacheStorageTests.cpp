#include "src/cache/NamespaceCacheStorage.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/core/mocks/MemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
#pragma pack(push, 1)

		struct NamespaceHistoryHeader {
			NamespaceId Id;
			uint64_t Depth;
		};

		struct RootNamespaceHeader {
			Key Owner;
			Height LifetimeStart;
			Height LifetimeEnd;
			uint64_t NumChildren;
		};

		struct NamespaceData {
			NamespaceId Part1;
			NamespaceId Part2;
		};

#pragma pack(pop)
	}

	// region Save

	namespace {
		void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId id, uint64_t depth) {
			const auto& historyHeader = reinterpret_cast<const NamespaceHistoryHeader&>(*buffer.data());
			EXPECT_EQ(id, historyHeader.Id);
			EXPECT_EQ(depth, historyHeader.Depth);
		}

		void AssertRootHeader(
				const std::vector<uint8_t>& buffer,
				size_t offset,
				const Key& owner,
				Height lifetimeStart,
				Height lifetimeEnd,
				uint64_t numChildren) {
			auto message = "root header at " + std::to_string(offset);
			const auto& rootHeader = reinterpret_cast<const RootNamespaceHeader&>(*(buffer.data() + offset));
			EXPECT_EQ(owner, rootHeader.Owner) << message;
			EXPECT_EQ(lifetimeStart, rootHeader.LifetimeStart) << message;
			EXPECT_EQ(lifetimeEnd, rootHeader.LifetimeEnd) << message;
			EXPECT_EQ(numChildren, rootHeader.NumChildren) << message;
		}

		void AssertNamespaceData(const std::vector<uint8_t>& buffer, size_t offset, const std::vector<NamespaceData>& expected) {
			for (auto i = 0u; i < expected.size(); ++i) {
				const auto& data = reinterpret_cast<const NamespaceData&>(*(buffer.data() + offset + i * sizeof(NamespaceData)));
				auto hasMatch = std::any_of(expected.cbegin(), expected.cend(), [&data](const auto& expectedData) {
					return data.Part1 == expectedData.Part1 && data.Part2 == expectedData.Part2;
				});
				EXPECT_TRUE(hasMatch) << "actual at " << i << " (" << data.Part1 << ", " << data.Part2 << ")";
			}
		}
	}

	TEST(NamespaceCacheStorageTests, CannotSaveEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		state::RootNamespaceHistory history(NamespaceId(123));

		// Act:
		EXPECT_THROW(NamespaceCacheStorage::Save(std::make_pair(NamespaceId(), history), stream), catapult_runtime_error);
	}

	TEST(NamespaceCacheStorageTests, CanSaveHistoryWithDepthOneWithoutChildren) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(222, 333));

		// Act:
		NamespaceCacheStorage::Save(std::make_pair(NamespaceId(), history), stream);

		// Assert:
		ASSERT_EQ(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader), buffer.size());
		AssertHistoryHeader(buffer, NamespaceId(123), 1);
		AssertRootHeader(buffer, sizeof(NamespaceHistoryHeader), owner, Height(222), Height(333), 0);
	}

	TEST(NamespaceCacheStorageTests, CanSaveHistoryWithDepthOneWithChildren) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(222, 333));
		history.back().add(state::Namespace(test::CreatePath({ 123, 124 })));
		history.back().add(state::Namespace(test::CreatePath({ 123, 124, 125 })));
		history.back().add(state::Namespace(test::CreatePath({ 123, 126 })));

		// Act:
		NamespaceCacheStorage::Save(std::make_pair(NamespaceId(), history), stream);

		// Assert:
		ASSERT_EQ(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * sizeof(NamespaceData), buffer.size());
		AssertHistoryHeader(buffer, NamespaceId(123), 1);
		AssertRootHeader(buffer, sizeof(NamespaceHistoryHeader), owner, Height(222), Height(333), 3);

		auto offset = sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader);
		AssertNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(124), NamespaceId() },
			NamespaceData{ NamespaceId(124), NamespaceId(125) },
			NamespaceData{ NamespaceId(126), NamespaceId() },
		});
	}

	TEST(NamespaceCacheStorageTests, CanSaveHistoryWithDepthGreaterThanOneWithSameOwner) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(11, 111));
		history.push_back(owner, test::CreateLifetime(222, 333));
		history.back().add(state::Namespace(test::CreatePath({ 123, 124 })));
		history.back().add(state::Namespace(test::CreatePath({ 123, 124, 125 })));
		history.back().add(state::Namespace(test::CreatePath({ 123, 126 })));
		history.push_back(owner, test::CreateLifetime(444, 555));
		history.back().add(state::Namespace(test::CreatePath({ 123, 126, 129 })));

		// Act:
		NamespaceCacheStorage::Save(std::make_pair(NamespaceId(), history), stream);

		// Assert:
		ASSERT_EQ(sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * sizeof(NamespaceData), buffer.size());
		AssertHistoryHeader(buffer, NamespaceId(123), 3);

		auto offset = sizeof(NamespaceHistoryHeader);
		AssertRootHeader(buffer, offset, owner, Height(11), Height(111), 4);

		offset += sizeof(RootNamespaceHeader);
		AssertNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(124), NamespaceId() },
			NamespaceData{ NamespaceId(124), NamespaceId(125) },
			NamespaceData{ NamespaceId(126), NamespaceId() },
			NamespaceData{ NamespaceId(126), NamespaceId(129) }
		});

		offset += 4 * sizeof(NamespaceData);
		AssertRootHeader(buffer, offset, owner, Height(222), Height(333), 0);

		offset += sizeof(RootNamespaceHeader);
		AssertRootHeader(buffer, offset, owner, Height(444), Height(555), 0);
	}

	TEST(NamespaceCacheStorageTests, CanSaveHistoryWithDepthGreaterThanOneWithDifferentOwner) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		auto owner1 = test::CreateRandomOwner();
		auto owner2 = test::CreateRandomOwner();
		auto owner3 = test::CreateRandomOwner();
		state::RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner1, test::CreateLifetime(11, 111));
		history.push_back(owner2, test::CreateLifetime(222, 333));
		history.back().add(state::Namespace(test::CreatePath({ 123, 124 })));
		history.back().add(state::Namespace(test::CreatePath({ 123, 124, 125 })));
		history.back().add(state::Namespace(test::CreatePath({ 123, 126 })));
		history.push_back(owner3, test::CreateLifetime(444, 555));
		history.back().add(state::Namespace(test::CreatePath({ 123, 126 })));

		// Act:
		NamespaceCacheStorage::Save(std::make_pair(NamespaceId(), history), stream);

		// Assert:
		ASSERT_EQ(sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * sizeof(NamespaceData), buffer.size());
		AssertHistoryHeader(buffer, NamespaceId(123), 3);

		auto offset = sizeof(NamespaceHistoryHeader);
		AssertRootHeader(buffer, offset, owner1, Height(11), Height(111), 0);

		offset += sizeof(RootNamespaceHeader);
		AssertRootHeader(buffer, offset, owner2, Height(222), Height(333), 3);

		offset += sizeof(RootNamespaceHeader);
		AssertNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(124), NamespaceId() },
			NamespaceData{ NamespaceId(124), NamespaceId(125) },
			NamespaceData{ NamespaceId(126), NamespaceId() }
		});

		offset += 3 * sizeof(NamespaceData);
		AssertRootHeader(buffer, offset, owner3, Height(444), Height(555), 1);

		offset += sizeof(RootNamespaceHeader);
		AssertNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(126), NamespaceId() }
		});
	}

	// endregion

	// region Load

	namespace {
		void AssertRootNamespace(
				const state::RootNamespace& root,
				const Key& owner,
				Height lifetimeStart,
				Height lifetimeEnd,
				uint64_t numChildren) {
			auto message = "root " + std::to_string(root.id().unwrap());
			EXPECT_EQ(owner, root.owner()) << message;
			EXPECT_EQ(lifetimeStart, root.lifetime().Start) << message;
			EXPECT_EQ(lifetimeEnd, root.lifetime().End) << message;
			EXPECT_EQ(numChildren, root.size()) << message;
		}

		void WriteNamespaceData(std::vector<uint8_t>& buffer, size_t offset, const std::vector<NamespaceData>& expected) {
			for (auto i = 0u; i < expected.size(); ++i) {
				auto& data = reinterpret_cast<NamespaceData&>(*(buffer.data() + offset + i * sizeof(NamespaceData)));
				data = expected[i];
			}
		}
	}

	TEST(NamespaceCacheStorageTests, CannotLoadEmptyHistory) {
		// Arrange:
		NamespaceCache cache;
		auto delta = cache.createDelta();

		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 0 };
		mocks::MemoryStream stream("", buffer);

		// Act:
		EXPECT_THROW(NamespaceCacheStorage::Load(stream, *delta), catapult_runtime_error);
	}

	TEST(NamespaceCacheStorageTests, CanLoadHistoryWithDepthOneWithoutChildren) {
		// Arrange:
		NamespaceCache cache;
		auto delta = cache.createDelta();

		auto owner = test::CreateRandomOwner();
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 1 };
		auto offset = sizeof(NamespaceHistoryHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 0 };
		mocks::MemoryStream stream("", buffer);

		// Act:
		NamespaceCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 1, 1);

		ASSERT_TRUE(delta->contains(NamespaceId(123)));
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 0);
	}

	TEST(NamespaceCacheStorageTests, CanLoadHistoryWithDepthOneWithChildren) {
		// Arrange:
		NamespaceCache cache;
		auto delta = cache.createDelta();

		auto owner = test::CreateRandomOwner();
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * sizeof(NamespaceData));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 1 };
		auto offset = sizeof(NamespaceHistoryHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 3 };
		offset += sizeof(RootNamespaceHeader);
		WriteNamespaceData(buffer, offset, {
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(126), NamespaceId() }
		});
		mocks::MemoryStream stream("", buffer);

		// Act:
		NamespaceCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 4, 4);

		ASSERT_TRUE(delta->contains(NamespaceId(123)));
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 3);
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(124)).ns().parentId());
		EXPECT_EQ(NamespaceId(124), delta->get(NamespaceId(125)).ns().parentId());
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(126)).ns().parentId());
	}

	TEST(NamespaceCacheStorageTests, CanLoadHistoryWithDepthOneWithOutOfOrderChildren) {
		// Arrange:
		NamespaceCache cache;
		auto delta = cache.createDelta();

		auto owner = test::CreateRandomOwner();
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * sizeof(NamespaceData));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 1 };
		auto offset = sizeof(NamespaceHistoryHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 3 };
		offset += sizeof(RootNamespaceHeader);
		WriteNamespaceData(buffer, offset, {
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(126), NamespaceId() }
		});
		mocks::MemoryStream stream("", buffer);

		// Act:
		NamespaceCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 4, 4);

		ASSERT_TRUE(delta->contains(NamespaceId(123)));
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 3);
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(124)).ns().parentId());
		EXPECT_EQ(NamespaceId(124), delta->get(NamespaceId(125)).ns().parentId());
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(126)).ns().parentId());
	}

	TEST(NamespaceCacheStorageTests, CanLoadHistoryWithDepthGreaterThanOneWithSameOwner) {
		// Arrange:
		NamespaceCache cache;
		auto delta = cache.createDelta();

		auto owner = test::CreateRandomOwner();
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * sizeof(NamespaceData));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 3 };
		auto offset = sizeof(NamespaceHistoryHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(11), Height(111), 4 };
		offset += sizeof(RootNamespaceHeader);
		WriteNamespaceData(buffer, offset, {
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(126), NamespaceId() },
			{ NamespaceId(126), NamespaceId(129) }
		});
		offset += 4 * sizeof(NamespaceData);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 0 };
		offset += sizeof(RootNamespaceHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(444), Height(555), 0 };
		mocks::MemoryStream stream("", buffer);

		// Act:
		NamespaceCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 5, 15);

		ASSERT_TRUE(delta->contains(NamespaceId(123)));
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(444), Height(555), 4);
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(124)).ns().parentId());
		EXPECT_EQ(NamespaceId(124), delta->get(NamespaceId(125)).ns().parentId());
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(126)).ns().parentId());
		EXPECT_EQ(NamespaceId(126), delta->get(NamespaceId(129)).ns().parentId());

		// - check history (one back)
		delta->remove(NamespaceId(123));
		test::AssertCacheSizes(*delta, 1, 5, 10);
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 4);

		// - check history (two back)
		delta->remove(NamespaceId(123));
		test::AssertCacheSizes(*delta, 1, 5, 5);
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(11), Height(111), 4);
	}

	TEST(NamespaceCacheStorageTests, CanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner) {
		// Arrange:
		NamespaceCache cache;
		auto delta = cache.createDelta();

		auto owner1 = test::CreateRandomOwner();
		auto owner2 = test::CreateRandomOwner();
		auto owner3 = test::CreateRandomOwner();
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * sizeof(NamespaceData));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 3 };
		auto offset = sizeof(NamespaceHistoryHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner1, Height(11), Height(111), 0 };
		offset += sizeof(RootNamespaceHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner2, Height(222), Height(333), 3 };
		offset += sizeof(RootNamespaceHeader);
		WriteNamespaceData(buffer, offset, {
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(126), NamespaceId() }
		});
		offset += 3 * sizeof(NamespaceData);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner3, Height(444), Height(555), 1 };
		offset += sizeof(RootNamespaceHeader);
		WriteNamespaceData(buffer, offset, {
			{ NamespaceId(126), NamespaceId() }
		});
		mocks::MemoryStream stream("", buffer);

		// Act:
		NamespaceCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 2, 7);

		ASSERT_TRUE(delta->contains(NamespaceId(123)));
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner3, Height(444), Height(555), 1);
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(126)).ns().parentId());

		// - check history (one back)
		delta->remove(NamespaceId(123));
		test::AssertCacheSizes(*delta, 1, 4, 5);
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner2, Height(222), Height(333), 3);
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(124)).ns().parentId());
		EXPECT_EQ(NamespaceId(124), delta->get(NamespaceId(125)).ns().parentId());
		EXPECT_EQ(NamespaceId(123), delta->get(NamespaceId(126)).ns().parentId());

		// - check history (two back)
		delta->remove(NamespaceId(123));
		test::AssertCacheSizes(*delta, 1, 1, 1);
		AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner1, Height(11), Height(111), 0);
	}

	namespace {
		void AssertCannotLoadWithBadData(const NamespaceData& badData) {
			// Arrange:
			NamespaceCache cache;
			auto delta = cache.createDelta();

			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 2 * sizeof(NamespaceData));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 1 };
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 2 };
			offset += sizeof(RootNamespaceHeader);
			WriteNamespaceData(buffer, offset, {
				{ NamespaceId(126), NamespaceId() },
				badData
			});
			mocks::MemoryStream stream("", buffer);

			// Act:
			EXPECT_THROW(NamespaceCacheStorage::Load(stream, *delta), catapult_invalid_argument);
		}
	}

	TEST(NamespaceCacheStorageTests, CannotLoadHistoryWithAnyChildMissingParent) {
		// Assert: notice that 125 has parent 124, but 124 is not present
		AssertCannotLoadWithBadData({ NamespaceId(124), NamespaceId(125) });
	}

	TEST(NamespaceCacheStorageTests, CannotLoadHistoryWithRootChild) {
		// Assert: notice that this will be deserialized as root path { 123 }
		AssertCannotLoadWithBadData({ NamespaceId(), NamespaceId() });
	}

	// endregion
}}
