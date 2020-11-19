/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "src/state/MosaicRestrictionEntry.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region MosaicGlobalRestrictionTestTraits

	/// Basic mosaic global restriction test traits.
	template<typename TNotification>
	class MosaicGlobalRestrictionTestTraits {
	public:
		using NotificationType = TNotification;

	public:
		/// Creates default traits.
		MosaicGlobalRestrictionTestTraits()
				: m_mosaicId(GenerateRandomValue<MosaicId>())
				, m_referenceMosaicId(GenerateRandomValue<MosaicId>())
		{}

	public:
		/// Gets the random (unresolved) reference mosaic id.
		UnresolvedMosaicId referenceMosaicId() const {
			return UnresolveXor(m_referenceMosaicId);
		}

	public:
		/// Gets the unique entry key.
		Hash256 uniqueKey() {
			return state::CreateMosaicRestrictionEntryKey(m_mosaicId);
		}

		/// Creates an add notification with \a key and \a value.
		NotificationType createAddNotification(uint64_t key, uint64_t value) {
			auto restrictionType = model::MosaicRestrictionType::EQ;
			return NotificationType(UnresolveXor(m_mosaicId), UnresolveXor(m_referenceMosaicId), key, value, restrictionType);
		}

		/// Creates a delete notification with \a key.
		NotificationType createDeleteNotification(uint64_t key) {
			auto restrictionType = model::MosaicRestrictionType::NONE;
			return NotificationType(UnresolveXor(m_mosaicId), UnresolveXor(MosaicId()), key, 0, restrictionType);
		}

	public:
		/// Adds restriction with multiple values to cache as specified in \a keyValuePairs.
		void addRestrictionWithValuesToCache(
				cache::MosaicRestrictionCacheDelta& restrictionCache,
				const std::vector<std::pair<uint64_t, uint64_t>>& keyValuePairs) {
			state::MosaicGlobalRestriction restriction(m_mosaicId);
			for (const auto& pair : keyValuePairs)
				restriction.set(pair.first, { m_referenceMosaicId, pair.second, model::MosaicRestrictionType::EQ });

			restrictionCache.insert(state::MosaicRestrictionEntry(restriction));
		}

	public:
		/// Asserts \a entry is equal to the expected entry based on these traits with values specified in \a keyValuePairs.
		void assertEqual(const state::MosaicRestrictionEntry& entry, const std::vector<std::pair<uint64_t, uint64_t>>& keyValuePairs) {
			ASSERT_EQ(state::MosaicRestrictionEntry::EntryType::Global, entry.entryType());

			const auto& restriction = entry.asGlobalRestriction();
			EXPECT_EQ(m_mosaicId, restriction.mosaicId());

			EXPECT_EQ(keyValuePairs.size(), restriction.size());
			for (const auto& pair : keyValuePairs) {
				auto message = "key " + std::to_string(pair.second);

				state::MosaicGlobalRestriction::RestrictionRule rule;
				restriction.tryGet(pair.first, rule);

				EXPECT_EQ(m_referenceMosaicId, rule.ReferenceMosaicId) << message;
				EXPECT_EQ(pair.second, rule.RestrictionValue) << message;
				EXPECT_EQ(model::MosaicRestrictionType::EQ, rule.RestrictionType) << message;
			}
		}

	private:
		MosaicId m_mosaicId;
		MosaicId m_referenceMosaicId;
	};

	// endregion

	// region MosaicAddressRestrictionTestTraits

	/// Basic mosaic address restriction test traits.
	template<typename TNotification>
	class MosaicAddressRestrictionTestTraits {
	public:
		using NotificationType = TNotification;

	public:
		/// Creates default traits.
		MosaicAddressRestrictionTestTraits()
				: m_mosaicId(GenerateRandomValue<MosaicId>())
				, m_address(GenerateRandomByteArray<Address>())
				, m_unresolvedAddress(UnresolveXor(m_address))
		{}

	public:
		/// Gets the unique entry key.
		Hash256 uniqueKey() {
			return state::CreateMosaicRestrictionEntryKey(m_mosaicId, m_address);
		}

		/// Creates an add notification with \a key and \a value.
		NotificationType createAddNotification(uint64_t key, uint64_t value) {
			return NotificationType(UnresolveXor(m_mosaicId), key, m_unresolvedAddress, value);
		}

		/// Creates a delete notification with \a key.
		NotificationType createDeleteNotification(uint64_t key) {
			return createAddNotification(key, state::MosaicAddressRestriction::Sentinel_Removal_Value);
		}

	public:
		/// Adds restriction with multiple values to cache as specified in \a keyValuePairs.
		void addRestrictionWithValuesToCache(
				cache::MosaicRestrictionCacheDelta& restrictionCache,
				const std::vector<std::pair<uint64_t, uint64_t>>& keyValuePairs) {
			state::MosaicAddressRestriction restriction(m_mosaicId, m_address);
			for (const auto& pair : keyValuePairs)
				restriction.set(pair.first, pair.second);

			restrictionCache.insert(state::MosaicRestrictionEntry(restriction));
		}

	public:
		/// Asserts \a entry is equal to the expected entry based on these traits with values specified in \a keyValuePairs.
		void assertEqual(const state::MosaicRestrictionEntry& entry, const std::vector<std::pair<uint64_t, uint64_t>>& keyValuePairs) {
			ASSERT_EQ(state::MosaicRestrictionEntry::EntryType::Address, entry.entryType());

			const auto& restriction = entry.asAddressRestriction();
			EXPECT_EQ(m_mosaicId, restriction.mosaicId());
			EXPECT_EQ(m_address, restriction.address());

			EXPECT_EQ(keyValuePairs.size(), restriction.size());
			for (const auto& pair : keyValuePairs) {
				auto value = restriction.get(pair.first);

				EXPECT_EQ(pair.second, value) << "key " << pair.second;
			}
		}

	private:
		MosaicId m_mosaicId;
		Address m_address;
		UnresolvedAddress m_unresolvedAddress;
	};

	// endregion
}}
