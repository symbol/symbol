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

#include "MosaicEntry.h"
#include "catapult/utils/IntegerMath.h"

namespace catapult { namespace state {

	// region MosaicEntrySupplyMixin

	Amount MosaicEntrySupplyMixin::supply() const {
		return m_supply;
	}

	void MosaicEntrySupplyMixin::increaseSupply(Amount delta) {
		if (!utils::CheckedAdd(m_supply, delta))
			CATAPULT_THROW_INVALID_ARGUMENT_2("cannot increase mosaic supply above max (supply, delta)", m_supply, delta);
	}

	void MosaicEntrySupplyMixin::decreaseSupply(Amount delta) {
		if (delta > m_supply)
			CATAPULT_THROW_INVALID_ARGUMENT_2("cannot decrease mosaic supply below zero (supply, delta)", m_supply, delta);

		m_supply = m_supply - delta;
	}

	// endregion

	// region MosaicEntry

	MosaicEntry::MosaicEntry(MosaicId id, const MosaicDefinition& definition)
			: m_id(id)
			, m_definition(definition)
	{}

	MosaicId MosaicEntry::mosaicId() const {
		return m_id;
	}

	const MosaicDefinition& MosaicEntry::definition() const {
		return m_definition;
	}

	bool MosaicEntry::isActive(Height height) const {
		return m_definition.isActive(height);
	}

	// endregion
}}
