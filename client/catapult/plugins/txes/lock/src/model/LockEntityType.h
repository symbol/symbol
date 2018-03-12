#pragma once
#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
#include "catapult/model/EntityType.h"

namespace catapult { namespace model {

#endif

	/// Hash lock transaction.
	DEFINE_TRANSACTION_TYPE(Lock, Hash_Lock, 0x1);

	/// Secret lock transaction.
	DEFINE_TRANSACTION_TYPE(Lock, Secret_Lock, 0x2);

	/// Secret proof transaction.
	DEFINE_TRANSACTION_TYPE(Lock, Secret_Proof, 0x3);

#ifndef CUSTOM_ENTITY_TYPE_DEFINITION
}}
#endif
