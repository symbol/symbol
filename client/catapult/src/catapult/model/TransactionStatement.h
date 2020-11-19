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
#include "Receipt.h"
#include "ReceiptSource.h"

namespace catapult { namespace model {

	/// Collection of receipts scoped to a transaction.
	class TransactionStatement {
	public:
		/// Creates a statement around \a source.
		explicit TransactionStatement(const ReceiptSource& source);

	public:
		/// Gets the statement source.
		const ReceiptSource& source() const;

		/// Gets the number of attached receipts.
		size_t size() const;

		/// Gets the receipt at \a index.
		const Receipt& receiptAt(size_t index) const;

		/// Calculates a unique hash for this statement.
		Hash256 hash() const;

	public:
		/// Adds \a receipt to this transaction statement.
		void addReceipt(const Receipt& receipt);

	private:
		ReceiptSource m_source;
		std::vector<std::unique_ptr<Receipt>> m_receipts;
	};
}}
