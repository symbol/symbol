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
		Key Owner;
		Height LifetimeStart;
		Height LifetimeEnd;
		uint64_t NumChildren;
	};

	/// Namespace data.
	struct NamespaceData {
		NamespaceId Part1;
		NamespaceId Part2;
	};

#pragma pack(pop)

	/// Asserts that \a root has expected property values (\a owner, \a lifetimeStart, \a lifetimeEnd, \a numChildren)
	CATAPULT_INLINE
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

	/// Root namespace history load test suite.
	template<typename TTraits>
	class RootNamespaceHistoryLoadTests {
	private:
		using NamespaceHistoryHeader = typename TTraits::NamespaceHistoryHeader;

	private:
		static void WriteNamespaceData(std::vector<uint8_t>& buffer, size_t offset, const std::vector<NamespaceData>& expected) {
			for (auto i = 0u; i < expected.size(); ++i) {
				auto& data = reinterpret_cast<NamespaceData&>(*(buffer.data() + offset + i * sizeof(NamespaceData)));
				data = expected[i];
			}
		}

	public:
		static void AssertCannotLoadEmptyHistory() {
			// Arrange:
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 0);
			mocks::MockMemoryStream stream("", buffer);

			// Act + Assert:
			TTraits::template AssertCannotLoad<catapult_runtime_error>(stream);
		}

		static void AssertCanLoadHistoryWithDepthOneWithoutChildren() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
			auto offset = sizeof(NamespaceHistoryHeader);
			reinterpret_cast<RootNamespaceHeader&>(*(buffer.data() + offset)) = { owner, Height(222), Height(333), 0 };
			mocks::MockMemoryStream stream("", buffer);

			// Act:
			TTraits::AssertCanLoadHistoryWithDepthOneWithoutChildren(stream, owner);
		}

		static void AssertCanLoadHistoryWithDepthOneWithChildren() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * sizeof(NamespaceData));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
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

		static void AssertCannotLoadHistoryWithDepthOneWithOutOfOrderChildren() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 3 * sizeof(NamespaceData));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
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
			TTraits::AssertCannotLoadHistoryWithDepthOneOutOfOrderChildren(stream);
		}

		static void AssertCanLoadHistoryWithDepthGreaterThanOneWithSameOwner() {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * sizeof(NamespaceData));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 3);
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

		static void AssertCanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner() {
			// Arrange:
			auto owner1 = test::CreateRandomOwner();
			auto owner2 = test::CreateRandomOwner();
			auto owner3 = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + 3 * sizeof(RootNamespaceHeader) + 4 * sizeof(NamespaceData));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 3);
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

	private:
		static void AssertCannotLoadWithBadData(const NamespaceData& badData) {
			// Arrange:
			auto owner = test::CreateRandomOwner();
			std::vector<uint8_t> buffer(sizeof(NamespaceHistoryHeader) + sizeof(RootNamespaceHeader) + 2 * sizeof(NamespaceData));
			reinterpret_cast<NamespaceHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(123), 1);
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

#define DEFINE_ROOT_NAMESPACE_HISTORY_LOAD_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadEmptyHistory, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithoutChildren, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOneWithChildren, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadHistoryWithDepthOneWithOutOfOrderChildren, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOneWithSameOwner, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOneWithDifferentOwner, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadHistoryWithAnyChildMissingParent, POSTFIX) \
	MAKE_ROOT_NAMESPACE_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadHistoryWithRootChild, POSTFIX)
