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
#include "BlockStatement.h"

namespace catapult { namespace model {

	/// Collection of receipts scoped to a block.
	class BlockStatementBuilder {
	public:
		/// Creates a builder.
		BlockStatementBuilder();

	public:
		/// Gets the active source.
		const ReceiptSource& source() const;

		/// Sets the active \a source.
		void setSource(const ReceiptSource& source);

		/// Decrements the current primary source and removes all receipts associated with it.
		void popSource();

	public:
		/// Adds \a receipt to this builder.
		void addReceipt(const Receipt& receipt);

		/// Adds a resolution entry for resolving \a unresolved value to \a resolved value.
		void addResolution(const UnresolvedAddress& unresolved, const Address& resolved);

		/// Adds a resolution entry for resolving \a unresolved value to \a resolved value.
		void addResolution(UnresolvedMosaicId unresolved, MosaicId resolved);

	public:
		/// Builds a block statement.
		std::unique_ptr<BlockStatement> build();

	private:
		ReceiptSource m_activeSource;
		std::unique_ptr<BlockStatement> m_pStatement;
	};
}}
