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
#include "src/model/NamespaceConstants.h"

namespace catapult { namespace state {

	bool MosaicDefinition::isEternal() const {
		return Eternal_Artifact_Duration == m_properties.duration();
	}

	bool MosaicDefinition::isActive(Height height) const {
		return isEternal() || (height < Height(m_height.unwrap() + m_properties.duration().unwrap()) && height >= m_height);
	}

	bool MosaicDefinition::isExpired(Height height) const {
		return !isEternal() && m_height + Height(m_properties.duration().unwrap()) <= height;
	}
}}
