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

#include "MosaicDefinition.h"
#include "src/model/MosaicConstants.h"
#include "catapult/constants.h"

namespace catapult { namespace state {

	Height MosaicDefinition::startHeight() const {
		return m_startHeight;
	}

	const Address& MosaicDefinition::ownerAddress() const {
		return m_ownerAddress;
	}

	uint32_t MosaicDefinition::revision() const {
		return m_revision;
	}

	const model::MosaicProperties& MosaicDefinition::properties() const {
		return m_properties;
	}

	bool MosaicDefinition::isEternal() const {
		return Eternal_Artifact_Duration == m_properties.duration();
	}

	bool MosaicDefinition::isActive(Height height) const {
		return isEternal() || (height < Height(m_startHeight.unwrap() + m_properties.duration().unwrap()) && height >= m_startHeight);
	}

	bool MosaicDefinition::isExpired(Height height) const {
		return !isEternal() && m_startHeight + Height(m_properties.duration().unwrap()) <= height;
	}
}}
