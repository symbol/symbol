#pragma once
#include "catapult/model/FacilityCode.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace validators {

#define COPY_FACILITY_CODE(FACILITY_NAME) FACILITY_NAME = utils::to_underlying_type(model::FacilityCode::FACILITY_NAME)

	/// Possible validation facility codes.
	enum class FacilityCode : uint8_t {
		COPY_FACILITY_CODE(Aggregate),
		COPY_FACILITY_CODE(Core),
		COPY_FACILITY_CODE(Lock),
		COPY_FACILITY_CODE(Mosaic),
		COPY_FACILITY_CODE(Multisig),
		COPY_FACILITY_CODE(Namespace),
		COPY_FACILITY_CODE(Transfer),

		/// Chain facility code.
		Chain = 0xFF,
		/// Consumer facility code.
		Consumer = 0xFE,
		/// Extension facility code.
		Extension = 0x45,
		/// Hash facility code.
		Hash = 0x48,
		/// Signature facility code.
		Signature = 0x53
	};

#undef COPY_FACILITY_CODE
}}
