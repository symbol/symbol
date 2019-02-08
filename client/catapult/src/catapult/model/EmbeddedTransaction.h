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

#pragma once
#include "EntityBody.h"
#include "SizePrefixedEntity.h"
#include <iosfwd>

namespace catapult {
	namespace model {
		class NotificationSubscriber;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an embedded transaction (non-verifiable).
	struct EmbeddedTransaction : public EntityBody<SizePrefixedEntity> {};

#pragma pack(pop)

	/// Insertion operator for outputting \a transaction to \a out.
	std::ostream& operator<<(std::ostream& out, const EmbeddedTransaction& transaction);

	/// Checks the real size of \a transaction against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const EmbeddedTransaction& transaction, const TransactionRegistry& registry);

	/// Sends all notifications from \a transaction to \a sub.
	void PublishNotifications(const EmbeddedTransaction& transaction, NotificationSubscriber& sub);
}}
