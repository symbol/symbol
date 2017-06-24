#pragma once
#include "AccountInfo.h"
#include "Block.h"
#include "EntityRange.h"
#include "catapult/utils/ShortHash.h"

namespace catapult { namespace model {

	/// An entity range composed of blocks.
	using BlockRange = EntityRange<Block>;

	/// An entity range composed of transactions.
	using TransactionRange = EntityRange<Transaction>;

	/// An entity range composed of hashes.
	using HashRange = EntityRange<Hash256>;

	/// An entity range composed of short hashes.
	using ShortHashRange = EntityRange<utils::ShortHash>;

	/// An entity range composed of addresses.
	using AddressRange = EntityRange<Address>;

	/// An entity range composed of account infos.
	using AccountInfoRange = EntityRange<AccountInfo>;
}}
