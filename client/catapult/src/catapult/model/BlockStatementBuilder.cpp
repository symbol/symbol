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

#include "BlockStatementBuilder.h"

namespace catapult { namespace model {

	BlockStatementBuilder::BlockStatementBuilder()
			: m_activeSource({ 0, 0 })
			, m_pStatement(std::make_unique<BlockStatement>())
	{}

	const ReceiptSource& BlockStatementBuilder::source() const {
		return m_activeSource;
	}

	void BlockStatementBuilder::setSource(const ReceiptSource& source) {
		m_activeSource = source;
	}

	void BlockStatementBuilder::popSource() {
		if (0 == m_activeSource.PrimaryId)
			return;

		setSource({ m_activeSource.PrimaryId - 1, 0 });
		auto pTruncatedStatement = std::make_unique<BlockStatement>();
		DeepCopyTo(*pTruncatedStatement, *m_pStatement, m_activeSource.PrimaryId);
		m_pStatement = std::move(pTruncatedStatement);
	}

	void BlockStatementBuilder::addReceipt(const Receipt& receipt) {
		auto& statements = m_pStatement->TransactionStatements;
		auto iter = statements.find(m_activeSource);
		if (statements.end() == iter) {
			TransactionStatement statement(m_activeSource);
			statement.addReceipt(receipt);
			statements.emplace(m_activeSource, std::move(statement));
			return;
		}

		iter->second.addReceipt(receipt);
	}

	namespace {
		template<typename TResolutionStatements, typename TUnresolved, typename TResolved>
		void AddResolution(
				TResolutionStatements& statements,
				const ReceiptSource& source,
				const TUnresolved& unresolved,
				const TResolved& resolved) {
			auto iter = statements.find(unresolved);
			if (statements.end() == iter) {
				typename TResolutionStatements::value_type::second_type statement(unresolved);
				statement.addResolution(resolved, source);
				statements.emplace(unresolved, std::move(statement));
				return;
			}

			iter->second.addResolution(resolved, source);
		}
	}

	void BlockStatementBuilder::addResolution(const UnresolvedAddress& unresolved, const Address& resolved) {
		AddResolution(m_pStatement->AddressResolutionStatements, m_activeSource, unresolved, resolved);
	}

	void BlockStatementBuilder::addResolution(UnresolvedMosaicId unresolved, MosaicId resolved) {
		AddResolution(m_pStatement->MosaicResolutionStatements, m_activeSource, unresolved, resolved);
	}

	std::unique_ptr<BlockStatement> BlockStatementBuilder::build() {
		return std::move(m_pStatement);
	}
}}
