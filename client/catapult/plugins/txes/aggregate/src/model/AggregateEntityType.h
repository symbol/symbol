#pragma once
#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
#include "catapult/model/EntityType.h"

namespace catapult { namespace model {

#endif

	/// Fully complete aggregate transaction.
	DEFINE_TRANSACTION_TYPE(Aggregate, Aggregate_Complete, 0x1);

	/// Bonded aggregate transaction.
	DEFINE_TRANSACTION_TYPE(Aggregate, Aggregate_Bonded, 0x2);

#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
}}
#endif
