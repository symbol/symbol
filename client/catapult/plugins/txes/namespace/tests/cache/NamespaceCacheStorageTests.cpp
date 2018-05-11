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
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS NamespaceCacheStorageTests

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

	TEST(TEST_CLASS, CannotSaveEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		state::RootNamespaceHistory history(NamespaceId(123));

		// Act + Assert:
		EXPECT_THROW(NamespaceCacheStorage::Save(std::make_pair(NamespaceId(), history), stream), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSaveHistoryWithDepthOneWithoutChildren) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

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

	TEST(TEST_CLASS, CanSaveHistoryWithDepthOneWithChildren) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

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

	TEST(TEST_CLASS, CanSaveHistoryWithDepthGreaterThanOneWithSameOwner) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

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

	TEST(TEST_CLASS, CanSaveHistoryWithDepthGreaterThanOneWithDifferentOwner) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

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

		struct LoadTraits {
			template<typename TException>
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				EXPECT_THROW(NamespaceCacheStorage::Load(inputStream), TException);
			}

			static void AssertCanLoadHistoryWithDepthOneWithoutChildren(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadHistoryWithDepthOneWithChildren(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner(
					io::InputStream& inputStream,
					const Key& owner1,
					const Key& owner2,
					const Key& owner3);
		};

		struct LoadIntoTraits {
			template<typename TException>
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				NamespaceCache cache(CacheConfiguration{});
				auto delta = cache.createDelta();
				EXPECT_THROW(NamespaceCacheStorage::LoadInto(inputStream, *delta), TException);
			}

			static void AssertCanLoadHistoryWithDepthOneWithoutChildren(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadHistoryWithDepthOneWithChildren(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner(
					io::InputStream& inputStream,
					const Key& owner1,
					const Key& owner2,
					const Key& owner3);
		};
	}

#define LOAD_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Load) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_LoadInto) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadIntoTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	LOAD_TEST(CannotLoadEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 0 };
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::template AssertCannotLoad<catapult_runtime_error>(stream);
	}

	LOAD_TEST(CanLoadHistoryWithDepthOneWithoutChildren) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader));
		reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = { NamespaceId(123), 1 };
		auto offset = sizeof(NamespaceHistoryHeader);
		reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 0 };
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		TTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(stream, owner);
	}

	namespace {
		void LoadTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(io::InputStream& inputStream, const Key& owner) {
			// Act:
			auto history = NamespaceCacheStorage::Load(inputStream);

			// Assert:
			ASSERT_EQ(1u, history.historyDepth());

			AssertRootNamespace(history.back(), owner, Height(222), Height(333), 0);
		}

		void LoadIntoTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(io::InputStream& inputStream, const Key& owner) {
			// Act:
			NamespaceCache cache(CacheConfiguration{});
			auto delta = cache.createDelta();
			NamespaceCacheStorage::LoadInto(inputStream, *delta);
			cache.commit();

			// Assert:
			auto view = cache.createView();
			test::AssertCacheSizes(*view, 1, 1, 1);

			ASSERT_TRUE(view->contains(NamespaceId(123)));
			AssertRootNamespace(view->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 0);
		}
	}

	LOAD_TEST(CanLoadHistoryWithDepthOneWithChildren) {
		// Arrange:
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
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCanLoadHistoryWithDepthOneWithChildren(stream, owner);
	}

	LOAD_TEST(CanLoadHistoryWithDepthOneWithOutOfOrderChildren) {
		// Arrange:
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
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCanLoadHistoryWithDepthOneWithChildren(stream, owner);
	}

	namespace {
		void LoadTraits::AssertCanLoadHistoryWithDepthOneWithChildren(io::InputStream& inputStream, const Key& owner) {
			// Act:
			auto history = NamespaceCacheStorage::Load(inputStream);

			// Assert:
			ASSERT_EQ(1u, history.historyDepth());

			AssertRootNamespace(history.back(), owner, Height(222), Height(333), 3);
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
			EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());
		}

		void LoadIntoTraits::AssertCanLoadHistoryWithDepthOneWithChildren(io::InputStream& inputStream, const Key& owner) {
			// Act:
			NamespaceCache cache(CacheConfiguration{});
			auto delta = cache.createDelta();
			NamespaceCacheStorage::LoadInto(inputStream, *delta);
			cache.commit();

			// Assert:
			auto view = cache.createView();
			test::AssertCacheSizes(*view, 1, 4, 4);

			ASSERT_TRUE(view->contains(NamespaceId(123)));
			AssertRootNamespace(view->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 3);
			EXPECT_EQ(NamespaceId(123), view->get(NamespaceId(124)).ns().parentId());
			EXPECT_EQ(NamespaceId(124), view->get(NamespaceId(125)).ns().parentId());
			EXPECT_EQ(NamespaceId(123), view->get(NamespaceId(126)).ns().parentId());
		}
	}

	LOAD_TEST(CanLoadHistoryWithDepthGreaterThanOneWithSameOwner) {
		// Arrange:
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
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner(stream, owner);
	}

	namespace {
		void LoadTraits::AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner(io::InputStream& inputStream, const Key& owner) {
			// Act:
			auto history = NamespaceCacheStorage::Load(inputStream);

			// Assert:
			ASSERT_EQ(3u, history.historyDepth());

			AssertRootNamespace(history.back(), owner, Height(444), Height(555), 4);
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
			EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());
			EXPECT_EQ(NamespaceId(126), history.back().child(NamespaceId(129)).parentId());

			// - check history (one back)
			history.pop_back();
			AssertRootNamespace(history.back(), owner, Height(222), Height(333), 4);

			// - check history (two back)
			history.pop_back();
			AssertRootNamespace(history.back(), owner, Height(11), Height(111), 4);
		}

		void LoadIntoTraits::AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner(io::InputStream& inputStream, const Key& owner) {
			// Act:
			NamespaceCache cache(CacheConfiguration{});
			auto delta = cache.createDelta();
			NamespaceCacheStorage::LoadInto(inputStream, *delta);
			cache.commit();

			// Assert:
			auto view = cache.createView();
			test::AssertCacheSizes(*view, 1, 5, 15);

			ASSERT_TRUE(view->contains(NamespaceId(123)));
			AssertRootNamespace(view->get(NamespaceId(123)).root(), owner, Height(444), Height(555), 4);
			EXPECT_EQ(NamespaceId(123), view->get(NamespaceId(124)).ns().parentId());
			EXPECT_EQ(NamespaceId(124), view->get(NamespaceId(125)).ns().parentId());
			EXPECT_EQ(NamespaceId(123), view->get(NamespaceId(126)).ns().parentId());
			EXPECT_EQ(NamespaceId(126), view->get(NamespaceId(129)).ns().parentId());

			// - check history (one back)
			delta->remove(NamespaceId(123));
			test::AssertCacheSizes(*delta, 1, 5, 10);
			AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(222), Height(333), 4);

			// - check history (two back)
			delta->remove(NamespaceId(123));
			test::AssertCacheSizes(*delta, 1, 5, 5);
			AssertRootNamespace(delta->get(NamespaceId(123)).root(), owner, Height(11), Height(111), 4);
		}
	}

	LOAD_TEST(CanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner) {
		// Arrange:
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
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		TTraits::AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner(stream, owner1, owner2, owner3);
	}

	namespace {
		void LoadTraits::AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner(
				io::InputStream& inputStream,
				const Key& owner1,
				const Key& owner2,
				const Key& owner3) {
			// Act:
			auto history = NamespaceCacheStorage::Load(inputStream);

			// Assert:
			ASSERT_EQ(3u, history.historyDepth());

			AssertRootNamespace(history.back(), owner3, Height(444), Height(555), 1);
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

			// - check history (one back)
			history.pop_back();
			AssertRootNamespace(history.back(), owner2, Height(222), Height(333), 3);
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
			EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
			EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

			// - check history (two back)
			history.pop_back();
			AssertRootNamespace(history.back(), owner1, Height(11), Height(111), 0);
		}

		void LoadIntoTraits::AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner(
				io::InputStream& inputStream,
				const Key& owner1,
				const Key& owner2,
				const Key& owner3) {
			// Act:
			NamespaceCache cache(CacheConfiguration{});
			auto delta = cache.createDelta();
			NamespaceCacheStorage::LoadInto(inputStream, *delta);
			cache.commit();

			// Assert:
			auto view = cache.createView();
			test::AssertCacheSizes(*view, 1, 2, 7);

			ASSERT_TRUE(view->contains(NamespaceId(123)));
			AssertRootNamespace(view->get(NamespaceId(123)).root(), owner3, Height(444), Height(555), 1);
			EXPECT_EQ(NamespaceId(123), view->get(NamespaceId(126)).ns().parentId());

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
	}

	namespace {
		template<typename TTraits>
		void AssertCannotLoadWithBadData(const NamespaceData& badData) {
			// Arrange:
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
			mocks::MockMemoryStream stream("", buffer);

			// Act + Assert:
			TTraits::template AssertCannotLoad<catapult_invalid_argument>(stream);
		}
	}

	LOAD_TEST(CannotLoadHistoryWithAnyChildMissingParent) {
		// Assert: notice that 125 has parent 124, but 124 is not present
		AssertCannotLoadWithBadData<TTraits>({ NamespaceId(124), NamespaceId(125) });
	}

	LOAD_TEST(CannotLoadHistoryWithRootChild) {
		// Assert: notice that this will be deserialized as root path { 123 }
		AssertCannotLoadWithBadData<TTraits>({ NamespaceId(), NamespaceId() });
	}

	// endregion
}}
