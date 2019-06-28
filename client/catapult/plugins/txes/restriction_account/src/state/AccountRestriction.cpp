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

#include "AccountRestriction.h"

namespace catapult { namespace state {

	AccountRestriction::AccountRestriction(model::AccountRestrictionType restrictionType, size_t restrictionValueSize)
			: m_restrictionDescriptor(restrictionType | model::AccountRestrictionType::Block)
			, m_restrictionValueSize(restrictionValueSize)
	{}

	const AccountRestrictionDescriptor& AccountRestriction::descriptor() const {
		return m_restrictionDescriptor;
	}

	size_t AccountRestriction::valueSize() const {
		return m_restrictionValueSize;
	}

	const std::set<std::vector<uint8_t>>& AccountRestriction::values() const {
		return m_values;
	}

	bool AccountRestriction::contains(const std::vector<uint8_t>& value) const {
		return m_values.cend() != m_values.find(value);
	}

	bool AccountRestriction::canAllow(const model::RawAccountRestrictionModification& modification) const {
		return isOperationAllowed(modification, AccountRestrictionOperationType::Allow);
	}

	bool AccountRestriction::canBlock(const model::RawAccountRestrictionModification& modification) const {
		return isOperationAllowed(modification, AccountRestrictionOperationType::Block);
	}

	void AccountRestriction::allow(const model::RawAccountRestrictionModification& modification) {
		update(modification);
		m_restrictionDescriptor = m_values.empty()
				? AccountRestrictionDescriptor(m_restrictionDescriptor.restrictionType() | model::AccountRestrictionType::Block)
				: AccountRestrictionDescriptor(m_restrictionDescriptor.restrictionType());
	}

	void AccountRestriction::block(const model::RawAccountRestrictionModification& modification) {
		auto blockRestrictionType = m_restrictionDescriptor.restrictionType() | model::AccountRestrictionType::Block;
		m_restrictionDescriptor = AccountRestrictionDescriptor(blockRestrictionType);
		update(modification);
	}

	bool AccountRestriction::isOperationAllowed(
			const model::RawAccountRestrictionModification& modification,
			AccountRestrictionOperationType operationType) const {
		if (m_restrictionValueSize != modification.Value.size())
			CATAPULT_THROW_INVALID_ARGUMENT_2("invalid value size (expected / actual)", m_restrictionValueSize, modification.Value.size());

		auto validOperationType = AccountRestrictionOperationType::Allow == operationType
				? AccountRestrictionOperationType::Allow == m_restrictionDescriptor.operationType()
				: AccountRestrictionOperationType::Block == m_restrictionDescriptor.operationType();
		auto validContainment = model::AccountRestrictionModificationType::Add == modification.ModificationType
				? !contains(modification.Value)
				: contains(modification.Value);
		return (m_values.empty() || validOperationType) && validContainment;
	}

	void AccountRestriction::update(const model::RawAccountRestrictionModification& modification) {
		if (m_restrictionValueSize != modification.Value.size())
			CATAPULT_THROW_INVALID_ARGUMENT_2("invalid value size (expected / actual)", m_restrictionValueSize, modification.Value.size());

		if (model::AccountRestrictionModificationType::Add == modification.ModificationType)
			m_values.insert(modification.Value);
		else
			m_values.erase(modification.Value);
	}
}}
