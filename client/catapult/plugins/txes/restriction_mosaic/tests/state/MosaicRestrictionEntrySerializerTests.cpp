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

#include "src/state/MosaicRestrictionEntrySerializer.h"
#include "tests/test/MosaicRestrictionTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicRestrictionEntrySerializerTests

	namespace {
		// region raw structures

#pragma pack(push, 1)

		struct MosaicAddressRestrictionHeader {
			MosaicRestrictionEntry::EntryType EntryType;
			catapult::MosaicId MosaicId;
			catapult::Address Address;
			uint8_t NumValues;
		};

		struct MosaicGlobalRestrictionHeader {
			MosaicRestrictionEntry::EntryType EntryType;
			catapult::MosaicId MosaicId;
			uint8_t NumValues;
		};

		struct MosaicAddressRestrictionTuple {
			uint64_t Key;
			uint64_t Value;
		};

		struct MosaicGlobalRestrictionTuple {
			uint64_t Key;
			MosaicId ReferenceMosaicId;
			uint64_t RestrictionValue;
			model::MosaicRestrictionType RestrictionType;
		};

#pragma pack(pop)

		// endregion
	}

	// region traits

	namespace {
		struct AddressRestrictionTraits {
			using HeaderType = MosaicAddressRestrictionHeader;
			using TupleType = MosaicAddressRestrictionTuple;

			static MosaicRestrictionEntry GenerateRandomEntry(uint8_t count) {
				auto mosaicId = test::GenerateRandomValue<MosaicId>();
				auto address = test::GenerateRandomByteArray<Address>();
				MosaicAddressRestriction restriction(mosaicId, address);

				for (auto i = 0u; i < count; ++i)
					restriction.set(i + 100, test::Random());

				return MosaicRestrictionEntry(restriction);
			}

			static auto GetKeys(const MosaicRestrictionEntry& entry) {
				return entry.asAddressRestriction().keys();
			}

			static void SetRandomValue(MosaicRestrictionEntry& entry, uint64_t key) {
				entry.asAddressRestriction().set(key, test::Random());
			}

			static void AssertHeader(const MosaicRestrictionEntry& entry, const MosaicAddressRestrictionHeader& header) {
				auto restriction = entry.asAddressRestriction();

				EXPECT_EQ(MosaicRestrictionEntry::EntryType::Address, header.EntryType);
				EXPECT_EQ(restriction.mosaicId(), header.MosaicId);
				EXPECT_EQ(restriction.address(), header.Address);
				EXPECT_EQ(restriction.keys().size(), header.NumValues);
			}

			static void AssertTuple(
					const MosaicRestrictionEntry& entry,
					uint64_t key,
					const MosaicAddressRestrictionTuple& tuple,
					const std::string& message) {
				auto restriction = entry.asAddressRestriction();

				EXPECT_EQ(restriction.get(key), tuple.Value) << message;
			}
		};

		struct GlobalRestrictionTraits {
		public:
			using HeaderType = MosaicGlobalRestrictionHeader;
			using TupleType = MosaicGlobalRestrictionTuple;

			static MosaicRestrictionEntry GenerateRandomEntry(uint8_t count) {
				auto mosaicId = test::GenerateRandomValue<MosaicId>();
				MosaicGlobalRestriction restriction(mosaicId);

				for (auto i = 0u; i < count; ++i)
					restriction.set(i + 100, CreateRandomRule());

				return MosaicRestrictionEntry(restriction);
			}

			static auto GetKeys(const MosaicRestrictionEntry& entry) {
				return entry.asGlobalRestriction().keys();
			}

			static void SetRandomValue(MosaicRestrictionEntry& entry, uint64_t key) {
				entry.asGlobalRestriction().set(key, CreateRandomRule());
			}

			static void AssertHeader(const MosaicRestrictionEntry& entry, const MosaicGlobalRestrictionHeader& header) {
				auto restriction = entry.asGlobalRestriction();

				EXPECT_EQ(MosaicRestrictionEntry::EntryType::Global, header.EntryType);
				EXPECT_EQ(restriction.mosaicId(), header.MosaicId);
				EXPECT_EQ(restriction.keys().size(), header.NumValues);
			}

			static void AssertTuple(
					const MosaicRestrictionEntry& entry,
					uint64_t key,
					const MosaicGlobalRestrictionTuple& tuple,
					const std::string& message) {
				auto restriction = entry.asGlobalRestriction();
				MosaicGlobalRestriction::RestrictionRule rule;
				restriction.tryGet(key, rule);

				EXPECT_EQ(rule.ReferenceMosaicId, tuple.ReferenceMosaicId) << message;
				EXPECT_EQ(rule.RestrictionValue, tuple.RestrictionValue) << message;
				EXPECT_EQ(rule.RestrictionType, tuple.RestrictionType) << message;
			}

		private:
			static MosaicGlobalRestriction::RestrictionRule CreateRandomRule() {
				MosaicGlobalRestriction::RestrictionRule rule;
				rule.ReferenceMosaicId = test::GenerateRandomValue<MosaicId>();
				rule.RestrictionValue = test::Random();
				rule.RestrictionType = model::MosaicRestrictionType::LT;
				return rule;
			}
		};
	}

#define RESTRICTION_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_AddressRestriction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressRestrictionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_GlobalRestriction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<GlobalRestrictionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region Save

	RESTRICTION_TRAITS_BASED_TEST(CanSaveEmptyEntry) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		auto entry = TTraits::GenerateRandomEntry(0);

		// Act:
		MosaicRestrictionEntrySerializer::Save(entry, outputStream);

		// Assert:
		ASSERT_EQ(sizeof(typename TTraits::HeaderType), buffer.size());

		const auto& header = reinterpret_cast<const typename TTraits::HeaderType&>(buffer[0]);
		TTraits::AssertHeader(entry, header);
	}

	RESTRICTION_TRAITS_BASED_TEST(CanSaveEntry) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		auto entry = TTraits::GenerateRandomEntry(3);

		// Act:
		MosaicRestrictionEntrySerializer::Save(entry, outputStream);

		// Assert:
		ASSERT_EQ(sizeof(typename TTraits::HeaderType) + 3 * sizeof(typename TTraits::TupleType), buffer.size());

		const auto& header = reinterpret_cast<const typename TTraits::HeaderType&>(buffer[0]);
		TTraits::AssertHeader(entry, header);

		auto i = 0u;
		const auto* pTuple = reinterpret_cast<const typename TTraits::TupleType*>(&buffer[sizeof(typename TTraits::HeaderType)]);
		for (auto key : TTraits::GetKeys(entry)) {
			auto tupleAlignedCopy = *pTuple; // not aligned so cannot be passed by reference
			auto message = "tuple at " + std::to_string(i++);
			EXPECT_EQ(key, tupleAlignedCopy.Key) << message;
			TTraits::AssertTuple(entry, key, tupleAlignedCopy, message);
			++pTuple;
		}
	}

	// endregion

	// region Save (Ordering)

	RESTRICTION_TRAITS_BASED_TEST(SavedRestrictionKeyValuePairsAreOrdered) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		auto entry = TTraits::GenerateRandomEntry(0);
		for (auto key : std::initializer_list<uint64_t>{ 123, 34, 57, 12 })
			TTraits::SetRandomValue(entry, key);

		// Act:
		MosaicRestrictionEntrySerializer::Save(entry, outputStream);

		// Assert:
		ASSERT_EQ(sizeof(typename TTraits::HeaderType) + 4 * sizeof(typename TTraits::TupleType), buffer.size());

		auto i = 0u;
		std::vector<uint64_t> expectedKeysOrdered{ 12, 34, 57, 123 };
		const auto* pTuple = reinterpret_cast<const typename TTraits::TupleType*>(&buffer[sizeof(typename TTraits::HeaderType)]);
		for (auto key : expectedKeysOrdered) {
			auto tupleAlignedCopy = *pTuple; // not aligned so cannot be passed by reference
			auto message = "tuple at " + std::to_string(i++);
			EXPECT_EQ(key, tupleAlignedCopy.Key) << message;
			++pTuple;
		}
	}

	// endregion

	// region Load (failure)

	RESTRICTION_TRAITS_BASED_TEST(CannotLoadWithInvalidEntryType) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		auto entry = TTraits::GenerateRandomEntry(3);

		MosaicRestrictionEntrySerializer::Save(entry, stream);
		stream.seek(0);

		// Sanity:
		ASSERT_EQ(sizeof(typename TTraits::HeaderType) + 3 * sizeof(typename TTraits::TupleType), buffer.size());

		// - corrupt the type
		auto& header = reinterpret_cast<typename TTraits::HeaderType&>(buffer[0]);
		header.EntryType = static_cast<MosaicRestrictionEntry::EntryType>(3);

		// Act + Assert:
		EXPECT_THROW(MosaicRestrictionEntrySerializer::Load(stream), catapult_invalid_argument);
	}

	// endregion

	// region Roundtrip

	namespace {
		template<typename TTraits>
		void AssertCanRoundtripEntry(uint8_t count) {
			// Arrange:
			auto originalEntry = TTraits::GenerateRandomEntry(count);

			// Act:
			auto result = test::RunRoundtripBufferTest<MosaicRestrictionEntrySerializer>(originalEntry);

			// Assert:
			test::AssertEqual(originalEntry, result);
		}
	}

	RESTRICTION_TRAITS_BASED_TEST(CanRoundtripEmptyEntry) {
		AssertCanRoundtripEntry<TTraits>(0);
	}

	RESTRICTION_TRAITS_BASED_TEST(CanRoundtripEntry) {
		AssertCanRoundtripEntry<TTraits>(3);
	}

	// endregion
}}
