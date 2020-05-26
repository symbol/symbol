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
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS RootNamespaceHistorySerializerTests

	using test::RootNamespaceHeader;
	using test::NamespaceData;
	using test::NamespaceDataWithAliasPayload;

	// region traits

	namespace {
		size_t GetExpectedChildGroupBufferSize(std::initializer_list<size_t> childGroup) {
			auto size = sizeof(RootNamespaceHeader);

			for (const auto& childDepth : childGroup)
				size += 2 + childDepth * sizeof(NamespaceId); // 1 (child depth) + 1 (alias type)

			return size;
		}

		struct FullTraits {
			using NamespaceHistoryHeader = test::NamespaceHistoryHeader;
			using Serializer = RootNamespaceHistorySerializer;

			static constexpr auto Has_Historical_Entries = true;

			static size_t GetExpectedBufferSize(std::initializer_list<std::initializer_list<size_t>> childGroups) {
				auto size = sizeof(NamespaceHistoryHeader);
				for (const auto& childGroup : childGroups)
					size += GetExpectedChildGroupBufferSize(childGroup);

				return size;
			}

			static NamespaceHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, uint64_t depth) {
				return { depth, namespaceId };
			}

			static void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, uint64_t depth) {
				const auto& historyHeader = reinterpret_cast<const NamespaceHistoryHeader&>(buffer[0]);
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

			static size_t GetExpectedBufferSize(std::initializer_list<std::initializer_list<size_t>> childGroups) {
				return sizeof(NamespaceHistoryHeader) + GetExpectedChildGroupBufferSize(*childGroups.begin());
			}

			static NamespaceHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, uint64_t) {
				return { namespaceId };
			}

			static void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, uint64_t) {
				const auto& historyHeader = reinterpret_cast<const NamespaceHistoryHeader&>(buffer[0]);
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
				const Address& owner,
				Height lifetimeStart,
				Height lifetimeEnd,
				uint64_t numChildren,
				const NamespaceAlias& alias = NamespaceAlias()) {
			auto message = "root header at " + std::to_string(offset);
			const auto& rootHeader = reinterpret_cast<const RootNamespaceHeader&>(buffer[offset]);
			EXPECT_EQ(owner, rootHeader.Owner) << message;
			EXPECT_EQ(lifetimeStart, rootHeader.LifetimeStart) << message;
			EXPECT_EQ(lifetimeEnd, rootHeader.LifetimeEnd) << message;

			EXPECT_EQ(alias.type(), rootHeader.AliasType) << message;
			if (AliasType::None == alias.type()) {
				// not aligned so cannot be passed by reference
				EXPECT_EQ(numChildren, static_cast<uint64_t>(rootHeader.NumChildren)) << message;
				return;
			}

			// when alias is present, RootNamespaceHeader::NumChildren is incorrect because it is after alias data
			auto* pAliasData = buffer.data() + offset + sizeof(RootNamespaceHeader) - sizeof(uint64_t);
			if (AliasType::Mosaic == alias.type()) {
				auto mosaicId = reinterpret_cast<const MosaicId&>(*pAliasData);
				EXPECT_EQ(alias.mosaicId(), mosaicId) << message;
				pAliasData += sizeof(MosaicId);
			} else {
				const auto& address = reinterpret_cast<const Address&>(*pAliasData);
				EXPECT_EQ(alias.address(), address) << message;
				pAliasData += Address::Size;
			}

			auto numActualChildren = reinterpret_cast<const uint64_t&>(*pAliasData);
			EXPECT_EQ(numChildren, numActualChildren) << message;
		}

		bool AreEqual(const NamespaceDataWithAliasPayload& lhs, const NamespaceData& rhs) {
			return lhs.Part1 == rhs.Part1 && lhs.Part2 == rhs.Part2 && lhs.Alias.type() == rhs.AliasType;
		}

		NamespaceData ReadNamespaceData(const uint8_t*& pData) {
			NamespaceData result;

			auto childDepth = *pData++;
			if (childDepth > 0) {
				result.Part1 = reinterpret_cast<const NamespaceId&>(*pData);
				pData += sizeof(NamespaceId);
			}

			if (childDepth > 1) {
				result.Part2 = reinterpret_cast<const NamespaceId&>(*pData);
				pData += sizeof(NamespaceId);
			}

			result.AliasType = static_cast<state::AliasType>(*pData++);
			return result;
		}

		void AssertOrderedNamespaceData(
				const std::vector<uint8_t>& buffer,
				size_t offset,
				const std::vector<NamespaceDataWithAliasPayload>& expected) {
			auto* pData = &buffer[offset];
			for (auto i = 0u; i < expected.size(); ++i) {
				auto namespaceData = ReadNamespaceData(pData);
				EXPECT_TRUE(AreEqual(expected[i], namespaceData))
						<< "actual at " << i << " (" << namespaceData.Part1 << ", " << namespaceData.Part2 << ")";
			}
		}

		void AssertNamespaceData(
				const std::vector<uint8_t>& buffer,
				size_t offset,
				const std::vector<NamespaceDataWithAliasPayload>& expected) {
			auto* pData = &buffer[offset];
			for (auto i = 0u; i < expected.size(); ++i) {
				// 1. check path and alias type
				auto namespaceData = ReadNamespaceData(pData);

				bool hasMatch = false;
				NamespaceAlias expectedAlias;
				for (const auto& expectedNamespaceData : expected) {
					if (!AreEqual(expectedNamespaceData, namespaceData))
						continue;

					hasMatch = true;
					expectedAlias = expectedNamespaceData.Alias;
					break;
				}

				std::ostringstream message;
				message << " at " << i << " (" << namespaceData.Part1 << ", " << namespaceData.Part2 << ")";
				EXPECT_TRUE(hasMatch) << "actual" << message.str();

				// 2. check alias payload
				switch (namespaceData.AliasType) {
				case AliasType::Mosaic:
					EXPECT_EQ(expectedAlias.mosaicId(), reinterpret_cast<const MosaicId&>(*pData)) << "mosaic alias" << message.str();
					pData += sizeof(MosaicId);
					break;

				case AliasType::Address:
					EXPECT_EQ(expectedAlias.address(), reinterpret_cast<const Address&>(*pData)) << "address alias" << message.str();
					pData += Address::Size;
					break;

				default:
					break;
				}
			}
		}
	}

	TEST(TEST_CLASS, CanSaveEmptyHistory_Historical) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		RootNamespaceHistory history(NamespaceId(123));

		// Act:
		FullTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(sizeof(FullTraits::NamespaceHistoryHeader), buffer.size());
		FullTraits::AssertHistoryHeader(buffer, NamespaceId(123), 0);
	}

	TEST(TEST_CLASS, CannotSaveEmptyHistory_NonHistorical) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		RootNamespaceHistory history(NamespaceId(123));

		// Act + Assert:
		EXPECT_THROW(NonHistoricalTraits::Serializer::Save(history, stream), catapult_runtime_error);
	}

	namespace {
		template<typename TTraits>
		void AssertCanSaveHistoryWithDepthOneWithoutChildren(const NamespaceAlias& alias, size_t aliasDataSize) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(owner, test::CreateLifetime(222, 333));
			history.back().setAlias(NamespaceId(123), alias);

			// Act:
			TTraits::Serializer::Save(history, stream);

			// Assert:
			ASSERT_EQ(TTraits::GetExpectedBufferSize({ {} }) + aliasDataSize, buffer.size());
			TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 1);
			AssertRootHeader(buffer, sizeof(typename TTraits::NamespaceHistoryHeader), owner, Height(222), Height(333), 0, alias);
		}
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithoutChildren) {
		AssertCanSaveHistoryWithDepthOneWithoutChildren<TTraits>(NamespaceAlias(), 0);
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithoutChildren_RootMosaicAlias) {
		AssertCanSaveHistoryWithDepthOneWithoutChildren<TTraits>(NamespaceAlias(MosaicId(567)), sizeof(MosaicId));
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithoutChildren_RootAddressAlias) {
		AssertCanSaveHistoryWithDepthOneWithoutChildren<TTraits>(NamespaceAlias(test::GenerateRandomByteArray<Address>()), Address::Size);
	}

	namespace {
		template<typename TTraits, typename TPrepareAliases>
		void AssertCanSaveHistoryWithDepthOneWithChildren(
				const std::vector<NamespaceDataWithAliasPayload>& expectedChildNamespaceData,
				size_t expectedAliasDataSize,
				TPrepareAliases prepareAliases) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(owner, test::CreateLifetime(222, 333));
			history.back().add(Namespace(test::CreatePath({ 123, 124 })));
			history.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
			history.back().add(Namespace(test::CreatePath({ 123, 126 })));
			prepareAliases(history);

			// Act:
			TTraits::Serializer::Save(history, stream);

			// Assert:
			ASSERT_EQ(TTraits::GetExpectedBufferSize({ { 1, 2, 1 } }) + expectedAliasDataSize, buffer.size());
			TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 1);
			AssertRootHeader(buffer, sizeof(typename TTraits::NamespaceHistoryHeader), owner, Height(222), Height(333), 3);

			auto offset = sizeof(typename TTraits::NamespaceHistoryHeader) + sizeof(RootNamespaceHeader);
			AssertNamespaceData(buffer, offset, expectedChildNamespaceData);
		}
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithChildren) {
		// Arrange:
		std::vector<NamespaceDataWithAliasPayload> expectedChildNamespaceData{
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(126), NamespaceId() }
		};

		// Assert:
		AssertCanSaveHistoryWithDepthOneWithChildren<TTraits>(expectedChildNamespaceData, 0, [](const auto&) {});
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOneWithChildren_WithAliases) {
		// Arrange:
		auto aliasedAddress = test::GenerateRandomByteArray<Address>();
		std::vector<NamespaceDataWithAliasPayload> expectedChildNamespaceData{
			{ NamespaceId(124), NamespaceId(), NamespaceAlias(MosaicId(444)) },
			{ NamespaceId(124), NamespaceId(125), NamespaceAlias(aliasedAddress) },
			{ NamespaceId(126), NamespaceId(), NamespaceAlias(MosaicId(987)) }
		};

		// Assert:
		auto aliasDataSize = 2 * sizeof(MosaicId) + Address::Size;
		AssertCanSaveHistoryWithDepthOneWithChildren<TTraits>(expectedChildNamespaceData, aliasDataSize, [&aliasedAddress](auto& history) {
			// Arrange:
			history.back().setAlias(NamespaceId(124), NamespaceAlias(MosaicId(444)));
			history.back().setAlias(NamespaceId(125), NamespaceAlias(aliasedAddress));
			history.back().setAlias(NamespaceId(126), NamespaceAlias(MosaicId(987)));
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
		mocks::MockMemoryStream stream(buffer);

		auto owner = test::CreateRandomOwner();
		RootNamespaceHistory history(NamespaceId(123));
		history.push_back(owner, test::CreateLifetime(222, 333));
		SeedRootNamespaceWithOutOfOrderChildren(history.back());

		// Sanity: child with namespace 3 element path is first in map
		const auto& children = history.back().children();
		EXPECT_EQ(3u, children.cbegin()->second.Path.size());

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		ASSERT_EQ(TTraits::GetExpectedBufferSize({ { 1, 2, 2, 2, 2, 2, 1, 2, 2 } }), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 1);
		AssertRootHeader(buffer, sizeof(typename TTraits::NamespaceHistoryHeader), owner, Height(222), Height(333), 9);

		auto offset = sizeof(typename TTraits::NamespaceHistoryHeader) + sizeof(RootNamespaceHeader);
		AssertOrderedNamespaceData(buffer, offset, {
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(121), },
			{ NamespaceId(124), NamespaceId(122), },
			{ NamespaceId(753), NamespaceId() },
			{ NamespaceId(753), NamespaceId(125) },
			{ NamespaceId(753), NamespaceId(126) },
			{ NamespaceId(753), NamespaceId(127) },
			{ NamespaceId(753), NamespaceId(128) },
			{ NamespaceId(753), NamespaceId(129) }
		});
	}

	namespace {
		template<typename TTraits, typename TPrepareAliases>
		void AssertCanSaveHistoryWithDepthGreaterThanOneSameOwner(
				const std::vector<NamespaceDataWithAliasPayload>& expectedChildNamespaceData,
				size_t expectedAliasDataSize,
				TPrepareAliases prepareAliases) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(owner, test::CreateLifetime(11, 111));
			history.push_back(owner, test::CreateLifetime(222, 333));
			history.back().add(Namespace(test::CreatePath({ 123, 124 })));
			history.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
			history.back().add(Namespace(test::CreatePath({ 123, 126 })));
			prepareAliases(history);
			history.push_back(owner, test::CreateLifetime(444, 555));
			history.back().add(Namespace(test::CreatePath({ 123, 126, 129 })));

			// Act:
			TTraits::Serializer::Save(history, stream);

			// Assert:
			ASSERT_EQ(TTraits::GetExpectedBufferSize({ { 1, 2, 1, 2 }, {}, {} }) + expectedAliasDataSize, buffer.size());
			TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 3);

			auto offset = sizeof(typename TTraits::NamespaceHistoryHeader);
			if (TTraits::Has_Historical_Entries) {
				AssertRootHeader(buffer, offset, owner, Height(11), Height(111), 4);
				offset += sizeof(RootNamespaceHeader);

				AssertNamespaceData(buffer, offset, expectedChildNamespaceData);
				offset += 4 * 2 + 6 * sizeof(NamespaceId) + expectedAliasDataSize;

				AssertRootHeader(buffer, offset, owner, Height(222), Height(333), 0);
				offset += sizeof(RootNamespaceHeader);

				AssertRootHeader(buffer, offset, owner, Height(444), Height(555), 0);
			} else {
				// - since no historical entries are present, all (active) child namespaces need to be serialized
				AssertRootHeader(buffer, offset, owner, Height(444), Height(555), 4);
				offset += sizeof(RootNamespaceHeader);

				AssertNamespaceData(buffer, offset, expectedChildNamespaceData);
			}
		}
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOneSameOwner) {
		// Arrange:
		std::vector<NamespaceDataWithAliasPayload> expectedChildNamespaceData{
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(126), NamespaceId() },
			{ NamespaceId(126), NamespaceId(129) }
		};

		// Assert:
		AssertCanSaveHistoryWithDepthGreaterThanOneSameOwner<TTraits>(expectedChildNamespaceData, 0, [](const auto&) {});
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOneSameOwner_WithAliases) {
		// Arrange:
		auto aliasedAddress = test::GenerateRandomByteArray<Address>();
		std::vector<NamespaceDataWithAliasPayload> expectedChildNamespaceData{
			{ NamespaceId(124), NamespaceId(), NamespaceAlias(MosaicId(444)) },
			{ NamespaceId(124), NamespaceId(125), NamespaceAlias(aliasedAddress) },
			{ NamespaceId(126), NamespaceId(), NamespaceAlias(MosaicId(987)) },
			{ NamespaceId(126), NamespaceId(129) }
		};

		// Assert:
		auto aliasDataSize = 2 * sizeof(MosaicId) + Address::Size;
		AssertCanSaveHistoryWithDepthGreaterThanOneSameOwner<TTraits>(expectedChildNamespaceData, aliasDataSize, [&aliasedAddress](
				auto& history) {
			// Arrange: aliases are set on the middle history entry
			history.back().setAlias(NamespaceId(124), NamespaceAlias(MosaicId(444)));
			history.back().setAlias(NamespaceId(125), NamespaceAlias(aliasedAddress));
			history.back().setAlias(NamespaceId(126), NamespaceAlias(MosaicId(987)));
		});
	}

	namespace {
		template<typename TTraits, typename TPrepareAliases>
		void AssertCanSaveHistoryWithDepthGreaterThanOneDifferentOwner(
				const std::vector<NamespaceDataWithAliasPayload>& expectedChildNamespaceData,
				size_t expectedAliasDataSize,
				TPrepareAliases prepareAliases) {
			// Arrange:
			std::vector<uint8_t> buffer;
			mocks::MockMemoryStream stream(buffer);

			auto owner1 = test::CreateRandomOwner();
			auto owner2 = test::CreateRandomOwner();
			auto owner3 = test::CreateRandomOwner();
			RootNamespaceHistory history(NamespaceId(123));
			history.push_back(owner1, test::CreateLifetime(11, 111));
			history.push_back(owner2, test::CreateLifetime(222, 333));
			history.back().add(Namespace(test::CreatePath({ 123, 124 })));
			history.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
			history.back().add(Namespace(test::CreatePath({ 123, 126 })));
			prepareAliases(history);
			history.push_back(owner3, test::CreateLifetime(444, 555));
			history.back().add(Namespace(test::CreatePath({ 123, 126 })));

			// Act:
			TTraits::Serializer::Save(history, stream);

			// Assert:
			if (!TTraits::Has_Historical_Entries)
				expectedAliasDataSize = 0; // aliases are only attached to second entry

			ASSERT_EQ(TTraits::GetExpectedBufferSize({ { 1 }, { 1, 2, 1 }, {} }) + expectedAliasDataSize, buffer.size());
			TTraits::AssertHistoryHeader(buffer, NamespaceId(123), 3);

			auto offset = sizeof(typename TTraits::NamespaceHistoryHeader);
			if (TTraits::Has_Historical_Entries) {
				AssertRootHeader(buffer, offset, owner1, Height(11), Height(111), 0);
				offset += sizeof(RootNamespaceHeader);

				AssertRootHeader(buffer, offset, owner2, Height(222), Height(333), 3);
				offset += sizeof(RootNamespaceHeader);

				AssertNamespaceData(buffer, offset, expectedChildNamespaceData);
				offset += 3 * 2 + 4 * sizeof(NamespaceId) + expectedAliasDataSize;
			}

			// - since namespace owner is different, all children are always serialized
			AssertRootHeader(buffer, offset, owner3, Height(444), Height(555), 1);
			offset += sizeof(RootNamespaceHeader);

			AssertNamespaceData(buffer, offset, {
				{ NamespaceId(126), NamespaceId() }
			});
		}
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOneDifferentOwner) {
		// Arrange:
		std::vector<NamespaceDataWithAliasPayload> expectedChildNamespaceData{
			{ NamespaceId(124), NamespaceId() },
			{ NamespaceId(124), NamespaceId(125) },
			{ NamespaceId(126), NamespaceId() }
		};

		// Assert:
		AssertCanSaveHistoryWithDepthGreaterThanOneDifferentOwner<TTraits>(expectedChildNamespaceData, 0, [](const auto&) {});
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOneDifferentOwner_WithAliases) {
		// Arrange:
		auto aliasedAddress = test::GenerateRandomByteArray<Address>();
		std::vector<NamespaceDataWithAliasPayload> expectedChildNamespaceData{
			{ NamespaceId(124), NamespaceId(), NamespaceAlias(MosaicId(444)) },
			{ NamespaceId(124), NamespaceId(125), NamespaceAlias(aliasedAddress) },
			{ NamespaceId(126), NamespaceId(), NamespaceAlias(MosaicId(987)) }
		};

		// Assert:
		auto aliasDataSize = 2 * sizeof(MosaicId) + Address::Size;
		AssertCanSaveHistoryWithDepthGreaterThanOneDifferentOwner<TTraits>(expectedChildNamespaceData, aliasDataSize, [&aliasedAddress](
				auto& history) {
			// Arrange: aliases are set on the middle history entry
			history.back().setAlias(NamespaceId(124), NamespaceAlias(MosaicId(444)));
			history.back().setAlias(NamespaceId(125), NamespaceAlias(aliasedAddress));
			history.back().setAlias(NamespaceId(126), NamespaceAlias(MosaicId(987)));
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

			static void AssertCanLoadEmptyHistory(io::InputStream& inputStream) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				EXPECT_EQ(0u, history.historyDepth());
				EXPECT_EQ(NamespaceId(123), history.id());
			}

			static void AssertCanLoadHistoryWithDepthOneWithoutChildren(
					io::InputStream& inputStream,
					const Address& owner,
					const NamespaceAlias& alias) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(1u, history.historyDepth());
				EXPECT_EQ(NamespaceId(123), history.id());

				test::AssertRootNamespace(history.back(), owner, Height(222), Height(333), 0, alias);
			}

			static void AssertCannotLoadHistoryWithDepthOneOutOfOrderChildren(io::InputStream& inputStream) {
				// Act + Assert:
				EXPECT_THROW(TTraits::Serializer::Load(inputStream), catapult_invalid_argument);
			}

			static void AssertCanLoadHistoryWithDepthOneWithChildren(
					io::InputStream& inputStream,
					const Address& owner,
					const std::vector<NamespaceAlias>& aliases) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(1u, history.historyDepth());
				EXPECT_EQ(NamespaceId(123), history.id());

				test::AssertRootNamespace(history.back(), owner, Height(222), Height(333), 3);
				EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
				EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
				EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

				// - check aliases
				ASSERT_EQ(3u, aliases.size());
				test::AssertEqualAlias(aliases[0], history.back().alias(NamespaceId(124)), "124");
				test::AssertEqualAlias(aliases[1], history.back().alias(NamespaceId(125)), "125");
				test::AssertEqualAlias(aliases[2], history.back().alias(NamespaceId(126)), "126");
			}

			static void AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner(
					io::InputStream& inputStream,
					const Address& owner,
					const std::vector<NamespaceAlias>& aliases) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(TTraits::Has_Historical_Entries ? 3u : 1u, history.historyDepth());
				EXPECT_EQ(NamespaceId(123), history.id());

				if (TTraits::Has_Historical_Entries) {
					test::AssertRootNamespace(history.back(), owner, Height(444), Height(555), 4);
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
					EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());
					EXPECT_EQ(NamespaceId(126), history.back().child(NamespaceId(129)).parentId());

					// - check aliases
					ASSERT_EQ(4u, aliases.size());
					test::AssertEqualAlias(aliases[0], history.back().alias(NamespaceId(124)), "124");
					test::AssertEqualAlias(aliases[1], history.back().alias(NamespaceId(125)), "125");
					test::AssertEqualAlias(aliases[2], history.back().alias(NamespaceId(126)), "126");
					test::AssertEqualAlias(aliases[3], history.back().alias(NamespaceId(129)), "129");

					// - check history (one back)
					history.pop_back();
					test::AssertRootNamespace(history.back(), owner, Height(222), Height(333), 4);

					// - check history (two back)
					history.pop_back();
				}

				// - entries are stored from oldest to newest, so non-historical serializer will deserialize first (oldest) entry
				test::AssertRootNamespace(history.back(), owner, Height(11), Height(111), 4);
			}

			static void AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner(
					io::InputStream& inputStream,
					const Address& owner1,
					const Address& owner2,
					const Address& owner3,
					const std::vector<NamespaceAlias>& aliases) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(TTraits::Has_Historical_Entries ? 3u : 1u, history.historyDepth());
				EXPECT_EQ(NamespaceId(123), history.id());

				if (TTraits::Has_Historical_Entries) {
					test::AssertRootNamespace(history.back(), owner3, Height(444), Height(555), 1);
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

					// - check aliases
					ASSERT_EQ(4u, aliases.size());
					test::AssertEqualAlias(aliases[3], history.back().alias(NamespaceId(126)), "126 :: 0");

					// - check history (one back)
					history.pop_back();
					test::AssertRootNamespace(history.back(), owner2, Height(222), Height(333), 3);
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(124)).parentId());
					EXPECT_EQ(NamespaceId(124), history.back().child(NamespaceId(125)).parentId());
					EXPECT_EQ(NamespaceId(123), history.back().child(NamespaceId(126)).parentId());

					// - check aliases
					test::AssertEqualAlias(aliases[0], history.back().alias(NamespaceId(124)), "124");
					test::AssertEqualAlias(aliases[1], history.back().alias(NamespaceId(125)), "125");
					test::AssertEqualAlias(aliases[2], history.back().alias(NamespaceId(126)), "126 :: 1");

					// - check history (two back)
					history.pop_back();
				}

				// - entries are stored from oldest to newest, so non-historical serializer will deserialize first (oldest) entry
				test::AssertRootNamespace(history.back(), owner1, Height(11), Height(111), 0);
			}
		};
	}

	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_TESTS(LoadTraits<FullTraits>,)
	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_NON_EMPTY_TESTS(LoadTraits<NonHistoricalTraits>, _NonHistorical)

	// endregion

	// region Roundtrip

	TEST(TEST_CLASS, CanRoundtripEmptyHistory_Historical) {
		// Arrange:
		RootNamespaceHistory originalHistory(NamespaceId(123));

		// Act:
		auto result = test::RunRoundtripBufferTest<RootNamespaceHistorySerializer>(originalHistory);

		// Assert:
		test::AssertHistoricalEqual(originalHistory, result);
	}

	namespace {
		template<typename TTraits>
		void AssertCanRoundtripHistoryWithDepthOneWithoutChildren(const NamespaceAlias& alias) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory originalHistory(NamespaceId(123));
			originalHistory.push_back(owner, test::CreateLifetime(222, 333));
			originalHistory.back().setAlias(NamespaceId(123), alias);

			// Act:
			auto result = test::RunRoundtripBufferTest<typename TTraits::Serializer>(originalHistory);

			// Assert:
			test::AssertHistoricalEqual(originalHistory, result);
		}
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthOneWithoutChildren) {
		AssertCanRoundtripHistoryWithDepthOneWithoutChildren<TTraits>(NamespaceAlias());
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthOneWithoutChildren_RootMosaicAlias) {
		AssertCanRoundtripHistoryWithDepthOneWithoutChildren<TTraits>(NamespaceAlias(MosaicId(567)));
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthOneWithoutChildren_RootAddressAlias) {
		AssertCanRoundtripHistoryWithDepthOneWithoutChildren<TTraits>(NamespaceAlias(test::GenerateRandomByteArray<Address>()));
	}

	namespace {
		template<typename TTraits, typename TPrepareAliases>
		void AssertCanRoundtripHistoryWithDepthOneWithChildren(TPrepareAliases prepareAliases) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory originalHistory(NamespaceId(123));
			originalHistory.push_back(owner, test::CreateLifetime(222, 333));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 124 })));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 126 })));
			prepareAliases(originalHistory);

			// Act:
			auto result = test::RunRoundtripBufferTest<typename TTraits::Serializer>(originalHistory);

			// Assert:
			test::AssertHistoricalEqual(originalHistory, result);
		}
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthOneWithChildren) {
		AssertCanRoundtripHistoryWithDepthOneWithChildren<TTraits>([](const auto&) {});
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthOneWithChildren_WithAliases) {
		AssertCanRoundtripHistoryWithDepthOneWithChildren<TTraits>([](auto& history) {
			// Arrange:
			history.back().setAlias(NamespaceId(124), NamespaceAlias(MosaicId(444)));
			history.back().setAlias(NamespaceId(125), NamespaceAlias(test::GenerateRandomByteArray<Address>()));
			history.back().setAlias(NamespaceId(126), NamespaceAlias(MosaicId(987)));
		});
	}

	namespace {
		template<typename TTraits, typename TPrepareAliases>
		void AssertCanRoundtripHistoryWithDepthGreaterThanOneSameOwner(TPrepareAliases prepareAliases) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			RootNamespaceHistory originalHistory(NamespaceId(123));
			originalHistory.push_back(owner, test::CreateLifetime(11, 111));
			originalHistory.push_back(owner, test::CreateLifetime(222, 333));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 124 })));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 126 })));
			prepareAliases(originalHistory);
			originalHistory.push_back(owner, test::CreateLifetime(444, 555));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 126, 129 })));

			// Act:
			auto result = test::RunRoundtripBufferTest<typename TTraits::Serializer>(originalHistory);

			// Assert:
			(TTraits::Has_Historical_Entries ? test::AssertHistoricalEqual : test::AssertNonHistoricalEqual)(originalHistory, result);
		}
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthGreaterThanOneSameOwner) {
		AssertCanRoundtripHistoryWithDepthGreaterThanOneSameOwner<TTraits>([](const auto&) {});
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthGreaterThanOneSameOwner_WithAliases) {
		AssertCanRoundtripHistoryWithDepthGreaterThanOneSameOwner<TTraits>([](auto& history) {
			// Arrange: aliases are set on the middle history entry
			history.back().setAlias(NamespaceId(124), NamespaceAlias(MosaicId(444)));
			history.back().setAlias(NamespaceId(125), NamespaceAlias(test::GenerateRandomByteArray<Address>()));
			history.back().setAlias(NamespaceId(126), NamespaceAlias(MosaicId(987)));
		});
	}

	namespace {
		template<typename TTraits, typename TPrepareAliases>
		void AssertCanRoundtripHistoryWithDepthGreaterThanOneDifferentOwner(TPrepareAliases prepareAliases) {
			// Arrange:
			auto owner1 = test::CreateRandomOwner();
			auto owner2 = test::CreateRandomOwner();
			auto owner3 = test::CreateRandomOwner();
			RootNamespaceHistory originalHistory(NamespaceId(123));
			originalHistory.push_back(owner1, test::CreateLifetime(11, 111));
			originalHistory.push_back(owner2, test::CreateLifetime(222, 333));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 124 })));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 124, 125 })));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 126 })));
			prepareAliases(originalHistory);
			originalHistory.push_back(owner3, test::CreateLifetime(444, 555));
			originalHistory.back().add(Namespace(test::CreatePath({ 123, 126 })));

			// Act:
			auto result = test::RunRoundtripBufferTest<typename TTraits::Serializer>(originalHistory);

			// Assert:
			(TTraits::Has_Historical_Entries ? test::AssertHistoricalEqual : test::AssertNonHistoricalEqual)(originalHistory, result);
		}
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthGreaterThanOneDifferentOwner) {
		AssertCanRoundtripHistoryWithDepthGreaterThanOneDifferentOwner<TTraits>([](const auto&) {});
	}

	SERIALIZER_TEST(CanRoundtripHistoryWithDepthGreaterThanOneDifferentOwner_WithAliases) {
		AssertCanRoundtripHistoryWithDepthGreaterThanOneDifferentOwner<TTraits>([](auto& history) {
			// Arrange: aliases are set on the middle history entry
			history.back().setAlias(NamespaceId(124), NamespaceAlias(MosaicId(444)));
			history.back().setAlias(NamespaceId(125), NamespaceAlias(test::GenerateRandomByteArray<Address>()));
			history.back().setAlias(NamespaceId(126), NamespaceAlias(MosaicId(987)));
		});
	}

	// endregion
}}
