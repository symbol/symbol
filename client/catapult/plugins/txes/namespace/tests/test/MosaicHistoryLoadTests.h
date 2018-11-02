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
#include "src/state/MosaicLevy.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

#pragma pack(push, 1)

	/// Mosaic history header.
	struct MosaicHistoryHeader {
		uint64_t Depth;
		catapult::NamespaceId NamespaceId;
		MosaicId Id;
	};

	/// Mosaic entry header.
	struct MosaicEntryHeader {
		catapult::Height Height;
		Key Owner;
		std::array<uint64_t, model::Num_Mosaic_Properties> PropertyValues;
		Amount Supply;
	};

#pragma pack(pop)

	/// Asserts that \a entry has expected property values (\a namespaceId, \a height, \a owner, \a propertyValues, \a supply).
	CATAPULT_INLINE
	void AssertMosaicEntry(
			const state::MosaicEntry& entry,
			NamespaceId namespaceId,
			Height height,
			const Key& owner,
			const decltype(MosaicEntryHeader::PropertyValues)& propertyValues,
			Amount supply) {
		// - entry
		auto message = "entry " + std::to_string(entry.mosaicId().unwrap());
		EXPECT_EQ(namespaceId, entry.namespaceId()) << message;

		// - definition
		const auto& definition = entry.definition();
		EXPECT_EQ(height, definition.height()) << message;
		EXPECT_EQ(owner, definition.owner()) << message;

		uint8_t i = 0;
		for (const auto& property : definition.properties()) {
			EXPECT_EQ(static_cast<model::MosaicPropertyId>(i), property.Id) << message << " property " << i;
			EXPECT_EQ(propertyValues[i], property.Value) << message << " property " << i;
			++i;
		}

		// - supply
		EXPECT_EQ(supply, entry.supply()) << message;
	}

	/// Mosaic history load test suite.
	template<typename TTraits>
	class MosaicHistoryLoadTests {
	private:
		using MosaicHistoryHeader = typename TTraits::MosaicHistoryHeader;

	public:
		static void AssertCannotLoadEmptyHistory() {
			// Arrange:
			std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader));
			reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(987), MosaicId(123), 0);
			mocks::MockMemoryStream inputStream("", buffer);

			// Act + Assert:
			TTraits::AssertCannotLoad(inputStream);
		}

		static void AssertCanLoadHistoryWithDepthOne() {
			// Arrange:
			auto owner = test::GenerateRandomData<Key_Size>();
			std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader) + sizeof(MosaicEntryHeader));
			reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(987), MosaicId(123), 1);
			auto offset = sizeof(MosaicHistoryHeader);
			reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(222), owner, { { 9, 8, 7 } }, Amount(786) };
			mocks::MockMemoryStream stream("", buffer);

			// Act + Assert:
			TTraits::AssertCanLoadWithDepthOne(stream, owner);
		}

		static void AssertCanLoadHistoryWithDepthGreaterThanOne() {
			// Arrange:
			auto owner1 = test::GenerateRandomData<Key_Size>();
			auto owner2 = test::GenerateRandomData<Key_Size>();
			std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader) + 3 * sizeof(MosaicEntryHeader));
			reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = TTraits::CreateHistoryHeader(NamespaceId(987), MosaicId(123), 3);
			auto offset = sizeof(MosaicHistoryHeader);
			reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(222), owner1, { { 9, 8, 7 } }, Amount(786) };
			offset += sizeof(MosaicEntryHeader);
			reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(321), owner2, { { 2, 5, 7 } }, Amount(999) };
			offset += sizeof(MosaicEntryHeader);
			reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(456), owner1, { { 1, 2, 4 } }, Amount(645) };
			mocks::MockMemoryStream stream("", buffer);

			// Act + Assert:
			TTraits::AssertCanLoadWithDepthGreaterThanOne(stream, owner1, owner2);
		}
	};
}}

#define MAKE_MOSAIC_HISTORY_LOAD_TEST(TRAITS_NAME, TEST_NAME, POSTFIX) \
	TEST(TEST_CLASS, TEST_NAME##POSTFIX) { test::MosaicHistoryLoadTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_MOSAIC_HISTORY_LOAD_TESTS(TRAITS_NAME, POSTFIX) \
	MAKE_MOSAIC_HISTORY_LOAD_TEST(TRAITS_NAME, CannotLoadEmptyHistory, POSTFIX) \
	MAKE_MOSAIC_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthOne, POSTFIX) \
	MAKE_MOSAIC_HISTORY_LOAD_TEST(TRAITS_NAME, CanLoadHistoryWithDepthGreaterThanOne, POSTFIX)
