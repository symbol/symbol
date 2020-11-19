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
#include "MosaicAddressRestriction.h"
#include "MosaicGlobalRestriction.h"
#include "catapult/plugins.h"
#include <memory>

namespace catapult { namespace state {

	// region MosaicRestrictionEntry

	/// Mosaic restriction entry.
	class PLUGIN_API_DEPENDENCY MosaicRestrictionEntry {
	public:
		/// Type of entry.
		enum class EntryType : uint8_t {
			/// Address restriction.
			Address,

			/// Global (mosaic) restriction.
			Global
		};

	public:
		/// Creates an entry around address \a restriction.
		explicit MosaicRestrictionEntry(const MosaicAddressRestriction& restriction);

		/// Creates an entry around global \a restriction.
		explicit MosaicRestrictionEntry(const MosaicGlobalRestriction& restriction);

		/// Copy constructor that makes a copy of \a entry.
		MosaicRestrictionEntry(const MosaicRestrictionEntry& entry);

	public:
		/// Assignment operator that makes a copy of \a entry.
		MosaicRestrictionEntry& operator=(const MosaicRestrictionEntry& entry);

	public:
		/// Gets the entry type.
		EntryType entryType() const;

		/// Gets the unique (composite) key.
		const Hash256& uniqueKey() const;

	public:
		/// Gets a const address restriction interface to this entry.
		const MosaicAddressRestriction& asAddressRestriction() const;

		/// Gets an address restriction interface to this entry.
		MosaicAddressRestriction& asAddressRestriction();

		/// Gets a const global restriction interface to this entry.
		const MosaicGlobalRestriction& asGlobalRestriction() const;

		/// Gets a global restriction interface to this entry.
		MosaicGlobalRestriction& asGlobalRestriction();

	private:
		Hash256 generateUniqueKey() const;

	private:
		std::shared_ptr<MosaicAddressRestriction> m_pAddressRestriction;
		std::shared_ptr<MosaicGlobalRestriction> m_pGlobalRestriction;
		Hash256 m_uniqueKey;
	};

	// endregion

	// region CreateMosaicRestrictionEntryKey

	/// Creates a mosaic restriction entry key from its component parts (\a mosaicId).
	Hash256 CreateMosaicRestrictionEntryKey(MosaicId mosaicId);

	/// Creates a mosaic restriction entry key from its component parts (\a mosaicId and \a address).
	Hash256 CreateMosaicRestrictionEntryKey(MosaicId mosaicId, const Address& address);

	// endregion
}}
