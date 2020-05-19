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

#pragma once
#include "catapult/utils/Casting.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

#pragma pack(push, 1)

	/// Namespace history header.
	struct NamespaceHistoryHeader {
		uint64_t Depth;
		catapult::NamespaceId NamespaceId;
	};

	/// Root namespace header.
	struct RootNamespaceHeader {
	public:
		RootNamespaceHeader(const Address& owner, Height lifetimeStart, Height lifetimeEnd, uint64_t numChildren)
				: Owner(owner)
				, LifetimeStart(lifetimeStart)
				, LifetimeEnd(lifetimeEnd)
				, AliasType(state::AliasType::None)
				, NumChildren(numChildren)
		{}

	public:
		Address Owner;
		Height LifetimeStart;
		Height LifetimeEnd;
		state::AliasType AliasType;
		uint64_t NumChildren;
	};

	/// Namespace data.
	struct NamespaceData {
	public:
		NamespaceData() = default;

		NamespaceData(NamespaceId part1, NamespaceId part2) : NamespaceData(part1, part2, state::AliasType::None)
		{}

		NamespaceData(NamespaceId part1, NamespaceId part2, state::AliasType aliasType)
				: Part1(part1)
				, Part2(part2)
				, AliasType(aliasType)
		{}

	public:
		NamespaceId Part1;
		NamespaceId Part2;
		state::AliasType AliasType;
	};

#pragma pack(pop)

	/// Namespace data with alias payload.
	struct NamespaceDataWithAliasPayload {
	public:
		NamespaceDataWithAliasPayload(NamespaceId part1, NamespaceId part2)
				: NamespaceDataWithAliasPayload(part1, part2, state::NamespaceAlias())
		{}

		NamespaceDataWithAliasPayload(NamespaceId part1, NamespaceId part2, const state::NamespaceAlias& alias)
				: Part1(part1)
				, Part2(part2)
				, Alias(alias)
		{}

	public:
		NamespaceId Part1;
		NamespaceId Part2;
		state::NamespaceAlias Alias;
	};

	/// Asserts that \a root has expected property values (\a owner, \a lifetimeStart, \a lifetimeEnd, \a numChildren)
	inline void AssertRootNamespace(
			const state::RootNamespace& root,
			const Address& owner,
			Height lifetimeStart,
			Height lifetimeEnd,
			uint64_t numChildren,
			const state::NamespaceAlias& alias = state::NamespaceAlias()) {
		auto message = "root " + std::to_string(root.id().unwrap());
		EXPECT_EQ(owner, root.ownerAddress()) << message;
		EXPECT_EQ(lifetimeStart, root.lifetime().Start) << message;
		EXPECT_EQ(lifetimeEnd, root.lifetime().End) << message;
		EXPECT_EQ(numChildren, root.size()) << message;

		auto rootAlias = root.alias(root.id());
		test::AssertEqualAlias(alias, rootAlias);
	}

	/// Root namespace history load test suite.
	template<typename TTraits>
	class RootNamespaceHistoryLoadTests {
	private:
		using NamespaceHistoryHeader = typename TTraits::NamespaceHistoryHeader;

	private:
		static size_t WriteNamespaceData(
				std::vector<uint8_t>& buffer,
				size_t offset,
				const std::vector<NamespaceDataWithAliasPayload>& seeds) {
			auto totalAliasDataSize = 0u;
			auto* pData = buffer.data() + offset;
			for (const auto& seed : seeds) {
				*pData++ = (NamespaceId() != seed.Part1 ? 1 : 0) + (NamespaceId() != seed.Part2 ? 1 : 0);

				if (NamespaceId() != seed.Part1) {
					reinterpret_cast<NamespaceId&>(*pData) = seed.Part1;
					pData += sizeof(NamespaceId);
				}

				if (NamespaceId() != seed.Part2) {
					reinterpret_cast<NamespaceId&>(*pData) = seed.Part2;
					pData += sizeof(NamespaceId);
				}

				*pData++ = utils::to_underlying_type(seed.Alias.type());

				auto aliasDataSize = 0u;
				switch (seed.Alias.type()) {
				case state::AliasType::Mosaic:
					reinterpret_cast<MosaicId&>(*pData) = seed.Alias.mosaicId();
					aliasDataSize = sizeof(MosaicId);
					break;

				case state::AliasType::Address:
					reinterpret_cast<Address&>(*pData) = seed.Alias.address();
					aliasDataSize = Address::Size;
					break;

				default:
					break;
				}

				pData += aliasDataSize;
				totalAliasDataSize += aliasDataSize;
			}

			return totalAliasDataSize;
		}

	public:
		static void AssertCanLoadEmptyHistory() {
			// Arrange:
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader));
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 0);
			mocks::MockMemoryStream stream(buffer);

			// Act:
			TTraits::AssertCanLoadEmptyHistory(stream);
		}

		static void AssertCannotLoadEmptyHistory() {
			// Arrange:
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader));
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 0);
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			TTraits::template AssertCannotLoad<catapult_runtime_error>(stream);
		}

	private:
		static size_t WriteDepthOneWithoutChildrenHeaders(std::vector<uint8_t>& buffer, const Address& owner, state::AliasType aliasType) {
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
			auto offset = sizeof(NamespaceHistoryHeader);

			auto& rootHeader = reinterpret_cast<RootNamespaceHeader&>(buffer[offset]);
			rootHeader = { owner, Height(222), Height(333), 0 };
			rootHeader.AliasType = aliasType;
			return offset;
		}

		template<typename T>
		static void WriteAliasData(std::vector<uint8_t>& buffer, size_t offset, const T& data) {
			offset += sizeof(RootNamespaceHeader) - sizeof(uint64_t);
			reinterpret_cast<T&>(buffer[offset]) = data;
			offset += sizeof(T);
			reinterpret_cast<uint64_t&>(buffer[offset]) = 0; // no children
		}

	public:
		static void AssertCanLoadHistoryWithDepthOneWithoutChildren() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader));
			WriteDepthOneWithoutChildrenHeaders(buffer, owner, state::AliasType::None);
			mocks::MockMemoryStream stream(buffer);

			// Act:
			TTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(stream, owner, state::NamespaceAlias());
		}

		static void AssertCanLoadHistoryWithDepthOneWithoutChildren_RootMosaicAlias() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + sizeof(MosaicId));
			auto offset = WriteDepthOneWithoutChildrenHeaders(buffer, owner, state::AliasType::Mosaic);

			WriteAliasData(buffer, offset, MosaicId(876));
			mocks::MockMemoryStream stream(buffer);

			// Act:
			TTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(stream, owner, state::NamespaceAlias(MosaicId(876)));
		}

		static void AssertCanLoadHistoryWithDepthOneWithoutChildren_RootAddressAlias() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + Address::Size);
			auto offset = WriteDepthOneWithoutChildrenHeaders(buffer, owner, state::AliasType::Address);

			auto addressAlias = test::GenerateRandomByteArray<Address>();
			WriteAliasData(buffer, offset, addressAlias);
			mocks::MockMemoryStream stream(buffer);

			// Act:
			TTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(stream, owner, state::NamespaceAlias(addressAlias));
		}

	private:
		static void AssertCanLoadHistoryWithDepthOneWithChildren(size_t aliasDataSize, const std::vector<state::NamespaceAlias>& aliases) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto namespaceDataSize = sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * 2 + 4 * sizeof(NamespaceId);
			std::vector<uint8_t> buffer(namespaceDataSize + aliasDataSize);
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner, Height(222), Height(333), 3 };
			offset += sizeof(RootNamespaceHeader);
			WriteNamespaceData(buffer, offset, {
				{ NamespaceId(124), NamespaceId(), aliases[0] },
				{ NamespaceId(124), NamespaceId(125), aliases[1] },
				{ NamespaceId(126), NamespaceId(), aliases[2] }
			});
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			TTraits::AssertCanLoadHistoryWithDepthOneWithChildren(stream, owner, aliases);
		}

	public:
		static void AssertCanLoadHistoryWithDepthOneWithChildren() {
			// Assert:
			AssertCanLoadHistoryWithDepthOneWithChildren(0, {
				state::NamespaceAlias(),
				state::NamespaceAlias(),
				state::NamespaceAlias()
			});
		}

		static void AssertCanLoadHistoryWithDepthOneWithChildren_WithAliases() {
			// Assert:
			auto aliasDataSize = 2 * sizeof(MosaicId) + Address::Size;
			AssertCanLoadHistoryWithDepthOneWithChildren(aliasDataSize, {
				state::NamespaceAlias(MosaicId(555)),
				state::NamespaceAlias(test::GenerateRandomByteArray<Address>()),
				state::NamespaceAlias(MosaicId(888))
			});
		}

		static void AssertCannotLoadHistoryWithDepthOneWithOutOfOrderChildren() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * 2 + 4 * sizeof(NamespaceId));
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner, Height(222), Height(333), 3 };
			offset += sizeof(RootNamespaceHeader);
			WriteNamespaceData(buffer, offset, {
				{ NamespaceId(124), NamespaceId(125) },
				{ NamespaceId(124), NamespaceId() },
				{ NamespaceId(126), NamespaceId() }
			});
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			TTraits::AssertCannotLoadHistoryWithDepthOneOutOfOrderChildren(stream);
		}

	private:
		static void AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner(
				size_t aliasDataSize,
				const std::vector<state::NamespaceAlias>& aliases) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			auto namespaceDataSize = sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * 2 + 6 * sizeof(NamespaceId);
			std::vector<uint8_t> buffer(namespaceDataSize + aliasDataSize);
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 3);
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner, Height(11), Height(111), 4 };
			offset += sizeof(RootNamespaceHeader);
			offset += WriteNamespaceData(buffer, offset, {
				{ NamespaceId(124), NamespaceId(), aliases[0] },
				{ NamespaceId(124), NamespaceId(125), aliases[1] },
				{ NamespaceId(126), NamespaceId(), aliases[2] },
				{ NamespaceId(126), NamespaceId(129), aliases[3] }
			});
			offset += 4 * 2 + 6 * sizeof(NamespaceId);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner, Height(222), Height(333), 0 };
			offset += sizeof(RootNamespaceHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner, Height(444), Height(555), 0 };
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			TTraits::AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner(stream, owner, aliases);
		}

	public:
		static void AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner() {
			// Assert:
			AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner(0, {
				state::NamespaceAlias(),
				state::NamespaceAlias(),
				state::NamespaceAlias(),
				state::NamespaceAlias()
			});
		}

		static void AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner_WithAliases() {
			// Assert:
			auto aliasDataSize = 2 * sizeof(MosaicId) + 2 * Address::Size;
			AssertCanLoadHistoryWithDepthGreaterThanOneSameOwner(aliasDataSize, {
				state::NamespaceAlias(MosaicId(555)),
				state::NamespaceAlias(test::GenerateRandomByteArray<Address>()),
				state::NamespaceAlias(MosaicId(888)),
				state::NamespaceAlias(test::GenerateRandomByteArray<Address>())
			});
		}

	private:
		static void AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner(
				size_t aliasDataSize,
				const std::vector<state::NamespaceAlias>& aliases) {
			// Arrange:
			auto owner1 = test::CreateRandomOwner();
			auto owner2 = test::CreateRandomOwner();
			auto owner3 = test::CreateRandomOwner();
			auto namespaceDataSize = sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * 2 + 5 * sizeof(NamespaceId);
			std::vector<uint8_t> buffer(namespaceDataSize + aliasDataSize);
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 3);
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner1, Height(11), Height(111), 0 };
			offset += sizeof(RootNamespaceHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner2, Height(222), Height(333), 3 };
			offset += sizeof(RootNamespaceHeader);
			offset += WriteNamespaceData(buffer, offset, {
				{ NamespaceId(124), NamespaceId(), aliases[0] },
				{ NamespaceId(124), NamespaceId(125), aliases[1] },
				{ NamespaceId(126), NamespaceId(), aliases[2] }
			});
			offset += 3 * 2 + 4 * sizeof(NamespaceId);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner3, Height(444), Height(555), 1 };
			offset += sizeof(RootNamespaceHeader);
			WriteNamespaceData(buffer, offset, {
				{ NamespaceId(126), NamespaceId(), aliases[3] }
			});
			mocks::MockMemoryStream stream(buffer);

			// Act:
			TTraits::AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner(stream, owner1, owner2, owner3, aliases);
		}

	public:
		static void AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner() {
			// Assert:
			AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner(0, {
				state::NamespaceAlias(),
				state::NamespaceAlias(),
				state::NamespaceAlias(),
				state::NamespaceAlias()
			});
		}

		static void AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner_WithAliases() {
			// Assert:
			auto aliasDataSize = 2 * sizeof(MosaicId) + 2 * Address::Size;
			AssertCanLoadHistoryWithDepthGreaterThanOneDifferentOwner(aliasDataSize, {
				state::NamespaceAlias(MosaicId(555)),
				state::NamespaceAlias(test::GenerateRandomByteArray<Address>()),
				state::NamespaceAlias(MosaicId(888)),
				state::NamespaceAlias(test::GenerateRandomByteArray<Address>())
			});
		}

	private:
		static void AssertCannotLoadWithBadData(const NamespaceData& badData) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 2 * 2 + 3 * sizeof(NamespaceId));
			reinterpret_cast<NamespaceHistoryHeader&>(buffer[0]) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(buffer[offset]) = { owner, Height(222), Height(333), 2 };
			offset += sizeof(RootNamespaceHeader);
			WriteNamespaceData(buffer, offset, {
				{ NamespaceId(126), NamespaceId() },
				{ badData.Part1, badData.Part2 }
			});
			mocks::MockMemoryStream stream(buffer);

			// Act + Assert:
			TTraits::template AssertCannotLoad<catapult_invalid_argument>(stream);
		}

	public:
		static void AssertCannotLoadHistoryWithAnyChildMissingParent() {
			// Assert: notice that 125 has parent 124, but 124 is not present
			AssertCannotLoadWithBadData({ NamespaceId(124), NamespaceId(125) });
		}

		static void AssertCannotLoadHistoryWithRootChild() {
			// Assert: notice that this will be deserialized as root path { 123 }
			AssertCannotLoadWithBadData({ NamespaceId(), NamespaceId() });
		}
	};
}}

#define MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, TEST_NAME, POSTFIX) \
	TEST(TEST_CLASS, TEST_NAME##POSTFIX) { test::RootNamespaceHistoryLoadTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_COMMON_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithoutChildren, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithoutChildren_RootMosaicAlias, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithoutChildren_RootAddressAlias, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithChildren, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithChildren_WithAliases, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadHistoryWithDepthOneWithOutOfOrderChildren, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOneSameOwner, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOneSameOwner_WithAliases, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOneDifferentOwner, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOneDifferentOwner_WithAliases, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadHistoryWithAnyChildMissingParent, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadHistoryWithRootChild, POSTFIX)

#define DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadEmptyHistory, POSTFIX) \
	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_COMMON_TESTS(TRAITS_NAME, POSTFIX)

#define DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_NON_EMPTY_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadEmptyHistory, POSTFIX) \
	DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_COMMON_TESTS(TRAITS_NAME, POSTFIX)

