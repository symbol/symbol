/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/model/Transaction.h"

namespace catapult { namespace io { class OutputStream; } }

namespace catapult { namespace test {

	/// Represents a transaction status notification.
	struct TransactionStatusNotification {
		/// Transaction.
		std::unique_ptr<model::Transaction> pTransaction;

		/// Transaction hash.
		Hash256 Hash;

		/// Transaction status.
		uint32_t Status;
	};

	/// Generates a random transaction status notification for a transaction with size \a transactionSize.
	TransactionStatusNotification GenerateRandomTransactionStatusNotification(uint32_t transactionSize);

	/// Writes \a notification to \a outputStream.
	void WriteTransactionStatusNotification(io::OutputStream& outputStream, const TransactionStatusNotification& notification);
}}
