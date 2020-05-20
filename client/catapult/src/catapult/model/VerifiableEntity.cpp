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

#include "VerifiableEntity.h"
#include "Address.h"
#include "Block.h"
#include "Transaction.h"

namespace catapult { namespace model {

	std::ostream& operator<<(std::ostream& out, const VerifiableEntity& entity) {
		out << entity.Type << " (v" << static_cast<uint16_t>(entity.Version) << ") with size " << entity.Size;
		return out;
	}

	Address GetSignerAddress(const VerifiableEntity& entity) {
		return PublicKeyToAddress(entity.SignerPublicKey, entity.Network);
	}

	bool IsSizeValid(const VerifiableEntity& entity, const TransactionRegistry& registry) {
		switch (ToBasicEntityType(entity.Type)) {
		case BasicEntityType::Block:
			return IsSizeValid(static_cast<const Block&>(entity), registry);
		case BasicEntityType::Transaction:
			return IsSizeValid(static_cast<const Transaction&>(entity), registry);
		default:
			return false;
		}
	}
}}
