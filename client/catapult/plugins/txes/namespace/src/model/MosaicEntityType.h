#pragma once
#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
#include "catapult/model/EntityType.h"

namespace catapult { namespace model {

#endif

	/// Mosaic definition transaction.
	DEFINE_TRANSACTION_TYPE(Mosaic, Mosaic_Definition, 0x1);

	/// Mosaic supply change transaction.
	DEFINE_TRANSACTION_TYPE(Mosaic, Mosaic_Supply_Change, 0x2);

	/// Mosaic levy change transaction.
	DEFINE_TRANSACTION_TYPE(Mosaic, Mosaic_Levy_Change, 0x3);

#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
}}
#endif
