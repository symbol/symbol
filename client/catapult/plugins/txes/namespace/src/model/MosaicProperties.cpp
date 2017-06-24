#include "MosaicProperties.h"

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

#define SET_REQUIRED(KEY, VALUE) values[utils::to_underlying_type(MosaicPropertyId::KEY)] = VALUE

		SET_REQUIRED(Flags, utils::to_underlying_type(header.Flags));
		SET_REQUIRED(Divisibility, header.Divisibility);

#undef SET_REQUIRED

		return MosaicProperties::FromValues(values);
	}
}}
