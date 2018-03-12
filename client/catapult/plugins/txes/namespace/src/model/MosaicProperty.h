#pragma once
#include "catapult/utils/BitwiseEnum.h"
#include <stdint.h>

namespace catapult { namespace model {

	/// The available mosaic property ids.
	enum class MosaicPropertyId : uint8_t {
		// region required properties

		/// The flags.
		Flags,
		/// The divisibility.
		Divisibility,

		// endregion

		// region optional properties

		/// The duration.
		Duration,

		// endregion

		Sentinel_Property_Id
	};

	/// Mosaic property flags.
	enum class MosaicFlags : uint8_t {
		/// No flags present.
		None = 0x00,

		/// Mosaic supply is mutable.
		Supply_Mutable = 0x01,

		/// Mosaic is transferable.
		Transferable = 0x02,

		/// Mosaic levy is mutable.
		Levy_Mutable = 0x04,

		/// All flags.
		All = 0x07
	};

	MAKE_BITWISE_ENUM(MosaicFlags)

#pragma pack(push, 1)

	/// Mosaic property composed of an id and a value.
	struct MosaicProperty {
	public:
		/// The mosaic property id.
		MosaicPropertyId Id;

		/// The mosaic property value.
		uint64_t Value;
	};

#pragma pack(pop)

	/// The number of available properties.
	constexpr size_t Num_Mosaic_Properties = utils::to_underlying_type(MosaicPropertyId::Sentinel_Property_Id);

	/// The index of first optional property.
	constexpr size_t First_Optional_Property = utils::to_underlying_type(MosaicPropertyId::Duration);
}}
