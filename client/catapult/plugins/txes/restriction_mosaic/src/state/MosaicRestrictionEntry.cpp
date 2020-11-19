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

#include "MosaicRestrictionEntry.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/exceptions.h"

namespace catapult { namespace state {

	// region MosaicRestrictionEntry

	MosaicRestrictionEntry::MosaicRestrictionEntry(const MosaicAddressRestriction& restriction)
			: m_pAddressRestriction(std::make_shared<MosaicAddressRestriction>(restriction))
			, m_uniqueKey(generateUniqueKey())
	{}

	MosaicRestrictionEntry::MosaicRestrictionEntry(const MosaicGlobalRestriction& restriction)
			: m_pGlobalRestriction(std::make_shared<MosaicGlobalRestriction>(restriction))
			, m_uniqueKey(generateUniqueKey())
	{}

	MosaicRestrictionEntry::MosaicRestrictionEntry(const MosaicRestrictionEntry& entry) {
		*this = entry;
	}

	MosaicRestrictionEntry& MosaicRestrictionEntry::operator=(const MosaicRestrictionEntry& entry) {
		m_pAddressRestriction = entry.m_pAddressRestriction
				? std::make_shared<MosaicAddressRestriction>(*entry.m_pAddressRestriction)
				: nullptr;
		m_pGlobalRestriction = entry.m_pGlobalRestriction
				? std::make_shared<MosaicGlobalRestriction>(*entry.m_pGlobalRestriction)
				: nullptr;
		m_uniqueKey = entry.m_uniqueKey;

		return *this;
	}

	MosaicRestrictionEntry::EntryType MosaicRestrictionEntry::entryType() const {
		return m_pAddressRestriction ? EntryType::Address : EntryType::Global;
	}

	const Hash256& MosaicRestrictionEntry::uniqueKey() const {
		return m_uniqueKey;
	}

	const MosaicAddressRestriction& MosaicRestrictionEntry::asAddressRestriction() const {
		if (!m_pAddressRestriction)
			CATAPULT_THROW_RUNTIME_ERROR("entry is not an address restriction");

		return *m_pAddressRestriction;
	}

	MosaicAddressRestriction& MosaicRestrictionEntry::asAddressRestriction() {
		if (!m_pAddressRestriction)
			CATAPULT_THROW_RUNTIME_ERROR("entry is not an address restriction");

		return *m_pAddressRestriction;
	}

	const MosaicGlobalRestriction& MosaicRestrictionEntry::asGlobalRestriction() const {
		if (!m_pGlobalRestriction)
			CATAPULT_THROW_RUNTIME_ERROR("entry is not an global restriction");

		return *m_pGlobalRestriction;
	}

	MosaicGlobalRestriction& MosaicRestrictionEntry::asGlobalRestriction() {
		if (!m_pGlobalRestriction)
			CATAPULT_THROW_RUNTIME_ERROR("entry is not an global restriction");

		return *m_pGlobalRestriction;
	}

	Hash256 MosaicRestrictionEntry::generateUniqueKey() const {
		return m_pAddressRestriction
				? CreateMosaicRestrictionEntryKey(m_pAddressRestriction->mosaicId(), m_pAddressRestriction->address())
				: CreateMosaicRestrictionEntryKey(m_pGlobalRestriction->mosaicId());
	}

	// endregion

	// region CreateMosaicRestrictionEntryKey

	Hash256 CreateMosaicRestrictionEntryKey(MosaicId mosaicId) {
		return CreateMosaicRestrictionEntryKey(mosaicId, Address());
	}

	Hash256 CreateMosaicRestrictionEntryKey(MosaicId mosaicId, const Address& address) {
		crypto::Sha3_256_Builder builder;
		builder.update({ reinterpret_cast<const uint8_t*>(&mosaicId), sizeof(MosaicId) });
		builder.update(address);

		Hash256 uniqueKey;
		builder.final(uniqueKey);
		return uniqueKey;
	}

	// endregion
}}
