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

#include "ObserverStatementBuilder.h"

namespace catapult { namespace observers {

	ObserverStatementBuilder::ObserverStatementBuilder() : m_pStatementBuilder(nullptr)
	{}

	ObserverStatementBuilder::ObserverStatementBuilder(model::BlockStatementBuilder& statementBuilder)
			: m_pStatementBuilder(&statementBuilder)
	{}

	model::ReceiptSource ObserverStatementBuilder::source() const {
		return m_pStatementBuilder ? m_pStatementBuilder->source() : model::ReceiptSource();
	}

	void ObserverStatementBuilder::setSource(const model::ReceiptSource& source) {
		if (!m_pStatementBuilder)
			return;

		m_pStatementBuilder->setSource(source);
	}

	void ObserverStatementBuilder::addReceipt(const model::Receipt& receipt) {
		if (!m_pStatementBuilder)
			return;

		m_pStatementBuilder->addReceipt(receipt);
	}

	model::ResolverContext Bind(const model::ResolverContext& resolverContext, model::BlockStatementBuilder& statementBuilder) {
		auto resolveAndCapture = [resolverContext, &statementBuilder](const auto& unresolved) {
			auto resolved = resolverContext.resolve(unresolved);
			if (0 != std::memcmp(&resolved, &unresolved, sizeof(resolved)))
				statementBuilder.addResolution(unresolved, resolved);

			return resolved;
		};

		return model::ResolverContext(resolveAndCapture, resolveAndCapture);
	}
}}
