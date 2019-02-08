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

#include "MosaicProperties.h"
#include "catapult/constants.h"

namespace catapult { namespace model {

	namespace {
		void SetDefaultValues(MosaicProperties::PropertyValuesContainer& values) {
			values[utils::to_underlying_type(MosaicPropertyId::Duration)] = Eternal_Artifact_Duration.unwrap();
		}
	}

	MosaicProperties ExtractAllProperties(const MosaicPropertiesHeader& header, const MosaicProperty* pProperties) {
		MosaicProperties::PropertyValuesContainer values;
		SetDefaultValues(values);

		const auto* pProperty = pProperties;
		for (auto i = 0u; i < header.Count; ++i, ++pProperty) {
			if (pProperty->Id >= MosaicPropertyId::Sentinel_Property_Id)
				continue;

			auto propertyId = utils::to_underlying_type(pProperty->Id);
			values[propertyId] = pProperty->Value;
		}

#define CATAPULT_SET_REQUIRED_PROPERTY(KEY, VALUE) values[utils::to_underlying_type(MosaicPropertyId::KEY)] = VALUE

		CATAPULT_SET_REQUIRED_PROPERTY(Flags, utils::to_underlying_type(header.Flags));
		CATAPULT_SET_REQUIRED_PROPERTY(Divisibility, header.Divisibility);

#undef CATAPULT_SET_REQUIRED_PROPERTY

		return MosaicProperties::FromValues(values);
	}
}}
