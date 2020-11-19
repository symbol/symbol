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
#include "catapult/model/BlockStatementBuilder.h"
#include "catapult/model/ResolverContext.h"

namespace catapult { namespace observers {

	/// Facade on top of a block statement builder that is accessible by observers.
	class ObserverStatementBuilder {
	public:
		/// Creates a default builder.
		ObserverStatementBuilder();

		/// Creates a builder around \a statementBuilder.
		ObserverStatementBuilder(model::BlockStatementBuilder& statementBuilder);

	public:
		/// Gets the active source.
		model::ReceiptSource source() const;

		/// Sets the active \a source.
		void setSource(const model::ReceiptSource& source);

	public:
		/// Adds \a receipt to this builder.
		void addReceipt(const model::Receipt& receipt);

	private:
		model::BlockStatementBuilder* m_pStatementBuilder;
	};

	/// Binds \a resolverContext to \a statementBuilder.
	model::ResolverContext Bind(const model::ResolverContext& resolverContext, model::BlockStatementBuilder& statementBuilder);
}}
