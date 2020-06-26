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
#include "catapult/plugins.h"
#include "catapult/types.h"

namespace catapult { namespace model { struct Transaction; } }

namespace catapult { namespace subscribers {

	/// Transaction status subscriber.
	class PLUGIN_API_DEPENDENCY TransactionStatusSubscriber {
	public:
		virtual ~TransactionStatusSubscriber() = default;

	public:
		/// Indicates \a transaction with \a hash completed with \a status.
		virtual void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) = 0;

		/// Flushes all queued data.
		virtual void flush() = 0;
	};
}}
