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

#include "src/state/RootNamespaceHistorySerializer.h"
#include "tests/test/RootNamespaceHistoryLoadTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS RootNamespaceHistorySerializerTests

	using test::RootNamespaceHeader;
	using test::NamespaceData;

	// region traits

	namespace {
		struct FullTraits {
			using NamespaceHistoryHeader = test::NamespaceHistoryHeader;
			using Serializer = RootNamespaceHistorySerializer;

			static constexpr auto Has_Historical_Entries = true;

			static size_t GetExpectedBufferSize(std::initializer_list<size_t> childCounts) {
				auto size = sizeof(NamespaceHistoryHeader);
				for (auto childCount : childCounts)
					size += sizeof(RootNamespaceHeader) + childCount * sizeof(NamespaceData);

				return size;
			}

			static NamespaceHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, uint64_t depth) {
				return { depth, namespaceId };
			}

			static void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, uint64_t depth) {
				const auto& historyHeader = reinterpret_cast<const NamespaceHistoryHeader&>(*buffer.data());
				EXPECT_EQ(namespaceId, historyHeader.NamespaceId);
				EXPECT_EQ(depth, historyHeader.Depth);
			}
		};

		struct NonHistoricalTraits {
			struct NamespaceHistoryHeader {
				catapult::NamespaceId NamespaceId;
			};
			using Serializer = RootNamespaceHistoryNonHistoricalSerializer;

			static constexpr auto Has_Historical_Entries = false;

			static size_t GetExpectedBufferSize(std::initializer_list<size_t> childCounts) {
				return sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + *childCounts.begin() * sizeof(NamespaceData);
			}

			static NamespaceHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, uint64_t) {
				return { namespaceId };
			}

			static void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, uint64_t) {
				const auto& historyHeader = reinterpret_cast<const NamespaceHistoryHeader&>(*buffer.data());
				EXPECT_EQ(namespaceId, historyHeader.NamespaceId);
			}
		};
	}

#define SERIALIZER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<FullTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonHistorical) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonHistoricalTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region Save

	namespace {
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

		bool AreEqual(const NamespaceData& lhs, const NamespaceData& rhs) {
			return lhs.Part1 == rhs.Part1 && lhs.Part2 == rhs.Part2;
		}

		void AssertOrderedNamespaceData(const std::vector<uint8_t>& buffer, size_t offset, const std::vector<NamespaceData>& expected) {
			for (auto i = 0u; i < expected.size(); ++i) {
				const auto& data = reinterpret_cast<const NamespaceData&>(*(buffer.data() + offset + i * sizeof(NamespaceData)));
				EXPECT_TRUE(AreEqual(expected[i], data)) << "actual at " << i << " (" << data.Part1 << ", " << data.Part2 << ")";
			}
		}

		void AssertNamespaceData(const std::vector<uint8_t>& buffer, size_t offset, const std::vector<NamespaceData>& expected) {
			for (auto i = 0u; i < expected.size(); ++i) {
				const auto& data = reinterpret_cast<const NamespaceData&>(*(buffer.data() + offset + i * sizeof(NamespaceData)));
				auto hasMatch = std::any_of(expected.cbegin(), expected.cend(), [&data](const auto& expectedData) {
					return AreEqual(data, expectedData);
				});
				EXPECT_TRUE(hasMatch) << "actual at " << i << " (" << data.Part1 << ", " << data.Part2 << ")";
			}
		}
	}

	SERIALIZER_TEST(CannotSaveEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		RootNamespaceHistory history(NamespaceId(123));

		// Act + Assert:
		EXPECT_THROW(TTraits::Serializer::Save(history, stream), catapult_runtime_error);
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithoutChildren) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(222, 333));

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(TTraits::GetExpectedBufferSize({ 0 }), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 1);
		AssertRootHeader(buffer, sizeof(typename TTraits::NamespaceHistoryHeader), owner, Height(222), Height(333), 0);
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithChildren) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(222, 333));
		history.back().add(Namespace(test::CreatePath({ 123, 124 })));
		history.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
		history.back().add(Namespace(test::CreatePath({ 123, 126 })));

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(TTraits::GetExpectedBufferSize({ 3 }), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 1);
		AssertRootHeader(buffer, sizeof(typename TTraits::NamespaceHistoryHeader), owner, Height(222), Height(333), 3);

		auto offset = sizeof(typename TTraits::NamespaceHistoryHeader) + sizeof(RootNamespaceHeader);
		AssertNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(124), NamespaceId() },
			NamespaceData{ NamespaceId(124), NamespaceId(125) },
			NamespaceData{ NamespaceId(126), NamespaceId() },
		});
	}

	namespace {
		void SeedRootNamespaceWithOutOfOrderChildren(RootNamespace& root) {
			// add more 3 element paths, to increase chance one of them will be first in unordered map
			std::vector<std::vector<NamespaceId::ValueType>> paths{
					{ 123, 753 },
					{ 123, 753, 129 },
					{ 123, 753, 127 },
					{ 123, 753, 128 },
					{ 123, 753, 125 },
					{ 123, 753, 126 },
					{ 123, 124 },
					{ 123, 124, 122 },
					{ 123, 124, 121 }
			};

			for (const auto& rawPath : paths)
				root.add(Namespace(test::CreatePath(rawPath)));
		}
	}

	SERIALIZER_TEST(SavingHistoryWithOutOfOrderChildrenSortsChildrenBeforeSaving) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(222, 333));
		SeedRootNamespaceWithOutOfOrderChildren(history.back());

		// Sanity: child with namespace 3 element path is first in map
		const auto& children = history.back().children();
		EXPECT_EQ(3u, children.cbegin()->second.size());

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(TTraits::GetExpectedBufferSize({ 9 }), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 1);
		AssertRootHeader(buffer, sizeof(typename TTraits::NamespaceHistoryHeader), owner, Height(222), Height(333), 9);

		auto offset = sizeof(typename TTraits::NamespaceHistoryHeader) + sizeof(RootNamespaceHeader);
		AssertOrderedNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(124), NamespaceId() },
			NamespaceData{ NamespaceId(124), NamespaceId(121), },
			NamespaceData{ NamespaceId(124), NamespaceId(122), },
			NamespaceData{ NamespaceId(753), NamespaceId() },
			NamespaceData{ NamespaceId(753), NamespaceId(125) },
			NamespaceData{ NamespaceId(753), NamespaceId(126) },
			NamespaceData{ NamespaceId(753), NamespaceId(127) },
			NamespaceData{ NamespaceId(753), NamespaceId(128) },
			NamespaceData{ NamespaceId(753), NamespaceId(129) }
		});
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOneWithSameOwner) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(11, 111));
		history.push_back(owner, test::CreateLifetime(222, 333));
		history.back().add(Namespace(test::CreatePath({ 123, 124 })));
		history.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
		history.back().add(Namespace(test::CreatePath({ 123, 126 })));
		history.push_back(owner, test::CreateLifetime(444, 555));
		history.back().add(Namespace(test::CreatePath({ 123, 126, 129 })));

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(TTraits::GetExpectedBufferSize({ 4, 0, 0 }), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 3);

		auto offset = sizeof(typename TTraits::NamespaceHistoryHeader);
		if (TTraits::Has_Historical_Entries) {
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
		} else {
			// - since no historical entries are present, all (active) child namespaces need to be serialized
			AssertRootHeader(buffer, offset, owner, Height(444), Height(555), 4);
			offset += sizeof(RootNamespaceHeader);

			AssertNamespaceData(buffer, offset, {
				NamespaceData{ NamespaceId(124), NamespaceId() },
				NamespaceData{ NamespaceId(124), NamespaceId(125) },
				NamespaceData{ NamespaceId(126), NamespaceId() },
				NamespaceData{ NamespaceId(126), NamespaceId(129) }
			});
		}
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOneWithDifferentOwner) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto owner1 = test::CreateRandomOwner();
		auto owner2 = test::CreateRandomOwner();
		auto owner3 = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner1, test::CreateLifetime(11, 111));
		history.push_back(owner2, test::CreateLifetime(222, 333));
		history.back().add(Namespace(test::CreatePath({ 123, 124 })));
		history.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
		history.back().add(Namespace(test::CreatePath({ 123, 126 })));
		history.push_back(owner3, test::CreateLifetime(444, 555));
		history.back().add(Namespace(test::CreatePath({ 123, 126 })));

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(TTraits::GetExpectedBufferSize({ 1, 3, 0 }), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 3);

		auto offset = sizeof(typename TTraits::NamespaceHistoryHeader);
		if (TTraits::Has_Historical_Entries) {
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
		}

		// - since namespace owner is different, all children are always serialized
		AssertRootHeader(buffer, offset, owner3, Height(444), Height(555), 1);
		offset += sizeof(RootNamespaceHeader);

		AssertNamespaceData(buffer, offset, {
			NamespaceData{ NamespaceId(126), NamespaceId() }
		});
	}

	// endregion

	// region Load

	namespace {
		template<typename TTraits>
		struct LoadTraits {
			using NamespaceHistoryHeader = typename TTraits::NamespaceHistoryHeader;

			static constexpr auto CreateHistoryHeader = TTraits::CreateHistoryHeader;

			template<typename TException>
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				EXPECT_THROW(TTraits::Serializer::Load(inputStream), TException);
			}

			static void AssertCanLoadHistoryWithDepthOneWithoutChildren(io::InputStream& inputStream, const Key& owner) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(1u, history.historyDepth());

				test::AssertRootNamespace(history.back(), owner, Height(222), Height(333), 0);
			}

			static void AssertCannotLoadHistoryWithDepthOneOutOfOrderChildren(io::InputStream& inputStream) {
				// Act + Assert:
				EXPECT_THROW(TTraits::Serializer::Load(inputStream), catapult_invalid_argument);
			}

			static void AssertCanLoadHistoryWithDepthOneWithChildren(io::InputStream& inputStream, const Key& owner) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(1u, history.historyDepth());

				test::AssertRootNamespace(history.back(), owner, Height(222), Height(333), 3);
				EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
				EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
				EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());
			}

			static void AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner(io::InputStream& inputStream, const Key& owner) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(TTraits::Has_Historical_Entries ? 3u : 1u, history.historyDepth());

				if (TTraits::Has_Historical_Entries) {
					test::AssertRootNamespace(history.back(), owner, Height(444), Height(555), 4);
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
					EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());
					EXPECT_EQ(NamespaceId(126), history.back().child(NamespaceId(129)).parentId());

					// - check history (one back)
					history.pop_back();
					test::AssertRootNamespace(history.back(), owner, Height(222), Height(333), 4);

					// - check history (two back)
					history.pop_back();
				}

				// - entries are stored from oldest to newest, so non-historical serializer will deserialize first (oldest) entry
				test::AssertRootNamespace(history.back(), owner, Height(11), Height(111), 4);
			}

			static void AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner(
					io::InputStream& inputStream,
					const Key& owner1,
					const Key& owner2,
					const Key& owner3) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(TTraits::Has_Historical_Entries ? 3u : 1u, history.historyDepth());

				if (TTraits::Has_Historical_Entries) {
					test::AssertRootNamespace(history.back(), owner3, Height(444), Height(555), 1);
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

					// - check history (one back)
					history.pop_back();
					test::AssertRootNamespace(history.back(), owner2, Height(222), Height(333), 3);
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
					EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

					// - check history (two back)
					history.pop_back();
				}

				// - entries are stored from oldest to newest, so non-historical serializer will deserialize first (oldest) entry
				test::AssertRootNamespace(history.back(), owner1, Height(11), Height(111), 0);
			}
		};
	}

	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_TESTS(LoadTraits<FullTraits>,)
	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_TESTS(LoadTraits<NonHistoricalTraits>, _NonHistorical)

	// endregion
}}
