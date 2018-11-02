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

#include "src/state/MosaicHistorySerializer.h"
#include "tests/test/MosaicHistoryLoadTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicHistorySerializerTests

	using test::MosaicEntryHeader;

	// region traits

	namespace {
		struct FullTraits {
			using MosaicHistoryHeader = test::MosaicHistoryHeader;
			using Serializer = MosaicHistorySerializer;

			static constexpr auto Has_Historical_Entries = true;

			static size_t GetExpectedBufferSize(size_t numEntries) {
				return sizeof(MosaicHistoryHeader) + numEntries * sizeof(MosaicEntryHeader);
			}

			static MosaicHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, MosaicId id, uint64_t depth) {
				return { depth, namespaceId, id };
			}

			static void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, MosaicId id, uint64_t depth) {
				const auto& historyHeader = reinterpret_cast<const MosaicHistoryHeader&>(*buffer.data());
				EXPECT_EQ(id, historyHeader.Id);
				EXPECT_EQ(namespaceId, historyHeader.NamespaceId);
				EXPECT_EQ(depth, historyHeader.Depth);
			}
		};

		struct NonHistoricalTraits {
			struct MosaicHistoryHeader {
				catapult::NamespaceId NamespaceId;
				MosaicId Id;
			};
			using Serializer = MosaicHistoryNonHistoricalSerializer;

			static constexpr auto Has_Historical_Entries = false;

			static size_t GetExpectedBufferSize(size_t numEntries) {
				return sizeof(MosaicHistoryHeader) + std::min<size_t>(1, numEntries) * sizeof(MosaicEntryHeader);
			}

			static MosaicHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, MosaicId id, uint64_t) {
				return { namespaceId, id };
			}

			static void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, MosaicId id, uint64_t) {
				const auto& historyHeader = reinterpret_cast<const MosaicHistoryHeader&>(*buffer.data());
				EXPECT_EQ(id, historyHeader.Id);
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
		std::unique_ptr<MosaicLevy> CreateMosaicLevy() {
			return std::make_unique<MosaicLevy>(
					MosaicId(9988),
					test::GenerateRandomData<Address_Decoded_Size>(),
					std::vector<MosaicLevyRule>());
		}

		model::MosaicProperties CreateMosaicProperties(uint64_t seed) {
			auto values = model::MosaicProperties::PropertyValuesContainer();

			uint8_t i = 0;
			for (auto& value : values) {
				value = i * i + seed;
				++i;
			}

			return model::MosaicProperties::FromValues(values);
		}

		void AssertEntryHeader(
				const std::vector<uint8_t>& buffer,
				size_t offset,
				Height height,
				const Key& owner,
				uint64_t propertiesSeed,
				Amount supply) {
			auto message = "entry header at " + std::to_string(offset);
			const auto& entryHeader = reinterpret_cast<const MosaicEntryHeader&>(*(buffer.data() + offset));

			// - definition
			EXPECT_EQ(height, entryHeader.Height) << message;
			EXPECT_EQ(owner, entryHeader.Owner) << message;
			for (auto i = 0u; i < model::Num_Mosaic_Properties; ++i)
				EXPECT_EQ(i * i + propertiesSeed, entryHeader.PropertyValues[i]) << message << " property " << i;

			// - supply
			EXPECT_EQ(supply, entryHeader.Supply) << message;
		}
	}

	SERIALIZER_TEST(CannotSaveEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		MosaicHistory history(NamespaceId(987), MosaicId(123));

		// Act + Assert:
		EXPECT_THROW(TTraits::Serializer::Save(history, stream), catapult_runtime_error);
	}

	SERIALIZER_TEST(CannotSaveHistoryContainingMosaicLevy) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// - add a levy
		auto definition = MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));

		MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition, Amount(111));
		history.back().setLevy(CreateMosaicLevy());

		// Act + Assert:
		EXPECT_THROW(TTraits::Serializer::Save(history, stream), catapult_runtime_error);
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthOne) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto definition = MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));
		MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition, Amount(111));

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		using MosaicHistoryHeader = typename TTraits::MosaicHistoryHeader;
		ASSERT_EQ(sizeof(MosaicHistoryHeader) + sizeof(MosaicEntryHeader), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(987), MosaicId(123), 1);
		AssertEntryHeader(buffer, sizeof(MosaicHistoryHeader), Height(888), definition.owner(), 17, Amount(111));
	}

	SERIALIZER_TEST(CanSaveHistoryWithDepthGreaterThanOne) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto definition1 = MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));
		auto definition2 = MosaicDefinition(Height(950), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(42));
		auto definition3 = MosaicDefinition(Height(999), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(11));
		MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition1, Amount(111));
		history.push_back(definition2, Amount(999));
		history.push_back(definition3, Amount(345));

		// Act:
		TTraits::Serializer::Save(history, stream);

		// Assert:
		using MosaicHistoryHeader = typename TTraits::MosaicHistoryHeader;
		ASSERT_EQ(TTraits::GetExpectedBufferSize(3), buffer.size());
		TTraits::AssertHistoryHeader(buffer, NamespaceId(987), MosaicId(123), 3);

		auto offset = sizeof(MosaicHistoryHeader);
		if (TTraits::Has_Historical_Entries) {
			AssertEntryHeader(buffer, offset, Height(888), definition1.owner(), 17, Amount(111));
			offset += sizeof(MosaicEntryHeader);

			AssertEntryHeader(buffer, offset, Height(950), definition2.owner(), 42, Amount(999));
			offset += sizeof(MosaicEntryHeader);
		}

		AssertEntryHeader(buffer, offset, Height(999), definition3.owner(), 11, Amount(345));
	}

	// endregion

	// region Load

	namespace {
		template<typename TTraits>
		struct LoadTraits {
			using MosaicHistoryHeader = typename TTraits::MosaicHistoryHeader;

			static constexpr auto CreateHistoryHeader = TTraits::CreateHistoryHeader;

			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				EXPECT_THROW(TTraits::Serializer::Load(inputStream), catapult_runtime_error);
			}

			static void AssertCanLoadWithDepthOne(io::InputStream& inputStream, const Key& owner) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(1u, history.historyDepth());

				test::AssertMosaicEntry(history.back(), NamespaceId(987), Height(222), owner, { { 9, 8, 7 } }, Amount(786));
			}

			static void AssertCanLoadWithDepthGreaterThanOne(io::InputStream& inputStream, const Key& owner1, const Key& owner2) {
				// Act:
				auto history = TTraits::Serializer::Load(inputStream);

				// Assert:
				ASSERT_EQ(TTraits::Has_Historical_Entries ? 3u : 1u, history.historyDepth());

				if (TTraits::Has_Historical_Entries) {
					test::AssertMosaicEntry(history.back(), NamespaceId(987), Height(456), owner1, { { 1, 2, 4 } }, Amount(645));

					// - check history (one back)
					history.pop_back();
					test::AssertMosaicEntry(history.back(), NamespaceId(987), Height(321), owner2, { { 2, 5, 7 } }, Amount(999));

					// - check history (two back)
					history.pop_back();
				}

				// - entries are stored from oldest to newest, so non-historical serializer will deserialize first (oldest) entry
				test::AssertMosaicEntry(history.back(), NamespaceId(987), Height(222), owner1, { { 9, 8, 7 } }, Amount(786));
			}
		};
	}

	DEFINE_MOSAIC_HISTORY_LOAD_TESTS(LoadTraits<FullTraits>,)
	DEFINE_MOSAIC_HISTORY_LOAD_TESTS(LoadTraits<NonHistoricalTraits>, _NonHistorical)

	// endregion
}}
