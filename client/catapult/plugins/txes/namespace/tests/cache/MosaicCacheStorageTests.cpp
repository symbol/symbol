#include "src/cache/MosaicCacheStorage.h"
#include "src/state/MosaicLevy.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/core/mocks/MemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		using PropertyValuesArray = std::array<uint64_t, model::Num_Mosaic_Properties>;

#pragma pack(push, 1)

		struct MosaicHistoryHeader {
			catapult::NamespaceId NamespaceId;
			MosaicId Id;
			uint64_t Depth;
		};

		struct MosaicEntryHeader {
			catapult::Height Height;
			Key Owner;
			PropertyValuesArray PropertyValues;
			Amount Supply;
		};

#pragma pack(pop)
	}

	// region Save

	namespace {
		std::unique_ptr<state::MosaicLevy> CreateMosaicLevy() {
			return std::make_unique<state::MosaicLevy>(
					MosaicId(9988),
					test::GenerateRandomData<Address_Decoded_Size>(),
					std::vector<state::MosaicLevyRule>());
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

		void AssertHistoryHeader(const std::vector<uint8_t>& buffer, NamespaceId namespaceId, MosaicId id, uint64_t depth) {
			const auto& historyHeader = reinterpret_cast<const MosaicHistoryHeader&>(*buffer.data());
			EXPECT_EQ(id, historyHeader.Id);
			EXPECT_EQ(namespaceId, historyHeader.NamespaceId);
			EXPECT_EQ(depth, historyHeader.Depth);
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

	TEST(MosaicCacheStorageTests, CannotSaveEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		state::MosaicHistory history(NamespaceId(987), MosaicId(123));

		// Act:
		EXPECT_THROW(MosaicCacheStorage::Save(std::make_pair(MosaicId(), history), stream), catapult_runtime_error);
	}

	TEST(MosaicCacheStorageTests, CannotSaveHistoryContainingMosaicLevy) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		// - add a levy
		auto definition = state::MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));

		state::MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition, Amount(111));
		history.back().setLevy(CreateMosaicLevy());

		// Act:
		EXPECT_THROW(MosaicCacheStorage::Save(std::make_pair(MosaicId(), history), stream), catapult_runtime_error);
	}

	TEST(MosaicCacheStorageTests, CanSaveHistoryWithDepthOne) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		auto definition = state::MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));
		state::MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition, Amount(111));

		// Act:
		MosaicCacheStorage::Save(std::make_pair(MosaicId(), history), stream);

		// Assert:
		ASSERT_EQ(sizeof(MosaicHistoryHeader) + sizeof(MosaicEntryHeader), buffer.size());
		AssertHistoryHeader(buffer, NamespaceId(987), MosaicId(123), 1);
		AssertEntryHeader(buffer, sizeof(MosaicHistoryHeader), Height(888), definition.owner(), 17, Amount(111));
	}

	TEST(MosaicCacheStorageTests, CanSaveHistoryWithDepthGreaterThanOne) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MemoryStream stream("", buffer);

		auto definition1 = state::MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));
		auto definition2 = state::MosaicDefinition(Height(950), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(42));
		auto definition3 = state::MosaicDefinition(Height(999), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(11));
		state::MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition1, Amount(111));
		history.push_back(definition2, Amount(999));
		history.push_back(definition3, Amount(345));

		// Act:
		MosaicCacheStorage::Save(std::make_pair(MosaicId(), history), stream);

		// Assert:
		ASSERT_EQ(sizeof(MosaicHistoryHeader) + 3 * sizeof(MosaicEntryHeader), buffer.size());
		AssertHistoryHeader(buffer, NamespaceId(987), MosaicId(123), 3);

		auto offset = sizeof(MosaicHistoryHeader);
		AssertEntryHeader(buffer, offset, Height(888), definition1.owner(), 17, Amount(111));

		offset += sizeof(MosaicEntryHeader);
		AssertEntryHeader(buffer, offset, Height(950), definition2.owner(), 42, Amount(999));

		offset += sizeof(MosaicEntryHeader);
		AssertEntryHeader(buffer, offset, Height(999), definition3.owner(), 11, Amount(345));
	}

	// endregion

	// region Load

	namespace {
		void AssertMosaicEntry(
				const state::MosaicEntry& entry,
				NamespaceId namespaceId,
				Height height,
				const Key& owner,
				const PropertyValuesArray& propertyValues,
				Amount supply) {
			// - entry
			auto message = "entry " + std::to_string(entry.mosaicId().unwrap());
			EXPECT_EQ(namespaceId, entry.namespaceId()) << message;

			// - definition
			const auto& definition = entry.definition();
			EXPECT_EQ(height, definition.height()) << message;
			EXPECT_EQ(owner, definition.owner()) << message;

			uint8_t i = 0;
			for (auto iter = definition.properties().cbegin(); definition.properties().cend() != iter; ++iter) {
				EXPECT_EQ(static_cast<model::MosaicPropertyId>(i), iter->Id) << message << " property " << i;
				EXPECT_EQ(propertyValues[i], iter->Value) << message << " property " << i;
				++i;
			}

			// - supply
			EXPECT_EQ(supply, entry.supply()) << message;
		}
	}

	TEST(MosaicCacheStorageTests, CannotLoadEmptyHistory) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader));
		reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = { NamespaceId(987), MosaicId(123), 0 };
		mocks::MemoryStream stream("", buffer);

		// Act:
		EXPECT_THROW(MosaicCacheStorage::Load(stream, *delta), catapult_runtime_error);
	}

	TEST(MosaicCacheStorageTests, CanLoadHistoryWithDepthOne) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		auto owner = test::GenerateRandomData<Key_Size>();
		std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader) + sizeof(MosaicEntryHeader));
		reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = { NamespaceId(987), MosaicId(123), 1 };
		auto offset = sizeof(MosaicHistoryHeader);
		reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(222), owner, { { 9, 8, 7 } }, Amount(786) };
		mocks::MemoryStream stream("", buffer);

		// Act:
		MosaicCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 1);

		ASSERT_TRUE(delta->contains(MosaicId(123)));
		AssertMosaicEntry(delta->get(MosaicId(123)), NamespaceId(987), Height(222), owner, { { 9, 8, 7 } }, Amount(786));
	}

	TEST(MosaicCacheStorageTests, CanLoadHistoryWithDepthGreaterThanOne) {
		// Arrange:
		MosaicCache cache;
		auto delta = cache.createDelta();

		auto owner1 = test::GenerateRandomData<Key_Size>();
		auto owner2 = test::GenerateRandomData<Key_Size>();
		std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader) + 3 * sizeof(MosaicEntryHeader));
		reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = { NamespaceId(987), MosaicId(123), 3 };
		auto offset = sizeof(MosaicHistoryHeader);
		reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(222), owner1, { { 9, 8, 7 } }, Amount(786) };
		offset += sizeof(MosaicEntryHeader);
		reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(321), owner2, { { 2, 5, 7 } }, Amount(999) };
		offset += sizeof(MosaicEntryHeader);
		reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(456), owner1, { { 1, 2, 4 } }, Amount(645) };
		mocks::MemoryStream stream("", buffer);

		// Act:
		MosaicCacheStorage::Load(stream, *delta);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 3);

		ASSERT_TRUE(delta->contains(MosaicId(123)));
		AssertMosaicEntry(delta->get(MosaicId(123)), NamespaceId(987), Height(456), owner1, { { 1, 2, 4 } }, Amount(645));

		// - check history (one back)
		delta->remove(MosaicId(123));
		test::AssertCacheSizes(*delta, 1, 2);
		AssertMosaicEntry(delta->get(MosaicId(123)), NamespaceId(987), Height(321), owner2, { { 2, 5, 7 } }, Amount(999));

		// - check history (two back)
		delta->remove(MosaicId(123));
		test::AssertCacheSizes(*delta, 1, 1);
		AssertMosaicEntry(delta->get(MosaicId(123)), NamespaceId(987), Height(222), owner1, { { 9, 8, 7 } }, Amount(786));
	}

	// endregion
}}
