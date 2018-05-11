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

#include "src/cache/MosaicCacheStorage.h"
#include "src/state/MosaicLevy.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicCacheStorageTests

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

	TEST(TEST_CLASS, CannotSaveEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		state::MosaicHistory history(NamespaceId(987), MosaicId(123));

		// Act + Assert:
		EXPECT_THROW(MosaicCacheStorage::Save(std::make_pair(MosaicId(), history), stream), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotSaveHistoryContainingMosaicLevy) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// - add a levy
		auto definition = state::MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), CreateMosaicProperties(17));

		state::MosaicHistory history(NamespaceId(987), MosaicId(123));
		history.push_back(definition, Amount(111));
		history.back().setLevy(CreateMosaicLevy());

		// Act + Assert:
		EXPECT_THROW(MosaicCacheStorage::Save(std::make_pair(MosaicId(), history), stream), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSaveHistoryWithDepthOne) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

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

	TEST(TEST_CLASS, CanSaveHistoryWithDepthGreaterThanOne) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

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
			for (const auto& property : definition.properties()) {
				EXPECT_EQ(static_cast<model::MosaicPropertyId>(i), property.Id) << message << " property " << i;
				EXPECT_EQ(propertyValues[i], property.Value) << message << " property " << i;
				++i;
			}

			// - supply
			EXPECT_EQ(supply, entry.supply()) << message;
		}

		struct LoadTraits {
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				EXPECT_THROW(MosaicCacheStorage::Load(inputStream), catapult_runtime_error);
			}

			static void AssertCanLoadWithDepthOne(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadWithDepthGreaterThanOne(io::InputStream& inputStream, const Key& owner1, const Key& owner2);
		};

		struct LoadIntoTraits {
			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				MosaicCache cache(CacheConfiguration{});
				auto delta = cache.createDelta();
				EXPECT_THROW(MosaicCacheStorage::LoadInto(inputStream, *delta), catapult_runtime_error);
			}

			static void AssertCanLoadWithDepthOne(io::InputStream& inputStream, const Key& owner);
			static void AssertCanLoadWithDepthGreaterThanOne(io::InputStream& inputStream, const Key& owner1, const Key& owner2);
		};
	}

#define LOAD_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Load) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_LoadInto) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LoadIntoTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	LOAD_TEST(CannotLoadEmptyHistory) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader));
		reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = { NamespaceId(987), MosaicId(123), 0 };
		mocks::MockMemoryStream inputStream("", buffer);

		// Act + Assert:
		TTraits::AssertCannotLoad(inputStream);
	}

	LOAD_TEST(CanLoadHistoryWithDepthOne) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		std::vector<uint8_t> buffer(sizeof(MosaicHistoryHeader) + sizeof(MosaicEntryHeader));
		reinterpret_cast<MosaicHistoryHeader&>(*buffer.data()) = { NamespaceId(987), MosaicId(123), 1 };
		auto offset = sizeof(MosaicHistoryHeader);
		reinterpret_cast<MosaicEntryHeader&>(*(buffer.data() + offset)) = { Height(222), owner, { { 9, 8, 7 } }, Amount(786) };
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCanLoadWithDepthOne(stream, owner);
	}

	namespace {
		void LoadTraits::AssertCanLoadWithDepthOne(io::InputStream& inputStream, const Key& owner) {
			// Act:
			auto history = MosaicCacheStorage::Load(inputStream);

			// Assert:
			ASSERT_EQ(1u, history.historyDepth());

			AssertMosaicEntry(history.back(), NamespaceId(987), Height(222), owner, { { 9, 8, 7 } }, Amount(786));
		}

		void LoadIntoTraits::AssertCanLoadWithDepthOne(io::InputStream& inputStream, const Key& owner) {
			// Act:
			MosaicCache cache(CacheConfiguration{});
			auto delta = cache.createDelta();
			MosaicCacheStorage::LoadInto(inputStream, *delta);
			cache.commit();

			// Assert:
			auto view = cache.createView();
			test::AssertCacheSizes(*view, 1, 1);

			ASSERT_TRUE(view->contains(MosaicId(123)));
			AssertMosaicEntry(view->get(MosaicId(123)), NamespaceId(987), Height(222), owner, { { 9, 8, 7 } }, Amount(786));
		}
	}

	LOAD_TEST(CanLoadHistoryWithDepthGreaterThanOne) {
		// Arrange:
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
		mocks::MockMemoryStream stream("", buffer);

		// Act + Assert:
		TTraits::AssertCanLoadWithDepthGreaterThanOne(stream, owner1, owner2);
	}

	namespace {
		void LoadTraits::AssertCanLoadWithDepthGreaterThanOne(io::InputStream& inputStream, const Key& owner1, const Key& owner2) {
			// Act:
			auto history = MosaicCacheStorage::Load(inputStream);

			// Assert:
			ASSERT_EQ(3u, history.historyDepth());

			AssertMosaicEntry(history.back(), NamespaceId(987), Height(456), owner1, { { 1, 2, 4 } }, Amount(645));

			// - check history (one back)
			history.pop_back();
			AssertMosaicEntry(history.back(), NamespaceId(987), Height(321), owner2, { { 2, 5, 7 } }, Amount(999));

			// - check history (two back)
			history.pop_back();
			AssertMosaicEntry(history.back(), NamespaceId(987), Height(222), owner1, { { 9, 8, 7 } }, Amount(786));
		}

		void LoadIntoTraits::AssertCanLoadWithDepthGreaterThanOne(io::InputStream& inputStream, const Key& owner1, const Key& owner2) {
			// Act:
			MosaicCache cache(CacheConfiguration{});
			auto delta = cache.createDelta();
			MosaicCacheStorage::LoadInto(inputStream, *delta);
			cache.commit();

			// Assert:
			auto view = cache.createView();
			test::AssertCacheSizes(*view, 1, 3);

			ASSERT_TRUE(view->contains(MosaicId(123)));
			AssertMosaicEntry(view->get(MosaicId(123)), NamespaceId(987), Height(456), owner1, { { 1, 2, 4 } }, Amount(645));

			// - check history (one back)
			delta->remove(MosaicId(123));
			test::AssertCacheSizes(*delta, 1, 2);
			AssertMosaicEntry(delta->get(MosaicId(123)), NamespaceId(987), Height(321), owner2, { { 2, 5, 7 } }, Amount(999));

			// - check history (two back)
			delta->remove(MosaicId(123));
			test::AssertCacheSizes(*delta, 1, 1);
			AssertMosaicEntry(delta->get(MosaicId(123)), NamespaceId(987), Height(222), owner1, { { 9, 8, 7 } }, Amount(786));
		}
	}

	// endregion
}}
