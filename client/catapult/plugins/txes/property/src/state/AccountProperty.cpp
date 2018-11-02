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

#include "AccountProperty.h"

namespace catapult { namespace state {

	AccountProperty::AccountProperty(model::PropertyType propertyType, size_t propertyValueSize)
			: m_propertyDescriptor(propertyType | model::PropertyType::Block)
			, m_propertyValueSize(propertyValueSize)
	{}

	const PropertyDescriptor& AccountProperty::descriptor() const {
		return m_propertyDescriptor;
	}

	size_t AccountProperty::propertyValueSize() const {
		return m_propertyValueSize;
	}

	const std::set<std::vector<uint8_t>>& AccountProperty::values() const {
		return m_values;
	}

	bool AccountProperty::contains(const std::vector<uint8_t>& value) const {
		return m_values.cend() != m_values.find(value);
	}

	bool AccountProperty::canAllow(const model::RawPropertyModification& modification) const {
		return isOperationAllowed(modification, OperationType::Allow);
	}

	bool AccountProperty::canBlock(const model::RawPropertyModification& modification) const {
		return isOperationAllowed(modification, OperationType::Block);
	}

	void AccountProperty::allow(const model::RawPropertyModification& modification) {
		update(modification);
		m_propertyDescriptor = m_values.empty()
				? PropertyDescriptor(m_propertyDescriptor.propertyType() | model::PropertyType::Block)
				: PropertyDescriptor(m_propertyDescriptor.propertyType());
	}

	void AccountProperty::block(const model::RawPropertyModification& modification) {
		m_propertyDescriptor = PropertyDescriptor(m_propertyDescriptor.propertyType() | model::PropertyType::Block);
		update(modification);
	}

	bool AccountProperty::isOperationAllowed(const model::RawPropertyModification& modification, OperationType operationType) const {
		if (m_propertyValueSize != modification.Value.size())
			CATAPULT_THROW_INVALID_ARGUMENT_2("invalid value size (expected / actual)", m_propertyValueSize, modification.Value.size());

		auto validOperationType = OperationType::Allow == operationType
				? OperationType::Allow == m_propertyDescriptor.operationType()
				: OperationType::Block == m_propertyDescriptor.operationType();
		auto validContainment = model::PropertyModificationType::Add == modification.ModificationType
				? !contains(modification.Value)
				: contains(modification.Value);
		return (m_values.empty() || validOperationType) && validContainment;
	}

	void AccountProperty::update(const model::RawPropertyModification& modification) {
		if (m_propertyValueSize != modification.Value.size())
			CATAPULT_THROW_INVALID_ARGUMENT_2("invalid value size (expected / actual)", m_propertyValueSize, modification.Value.size());

		if (model::PropertyModificationType::Add == modification.ModificationType)
			m_values.insert(modification.Value);
		else
			m_values.erase(modification.Value);
	}
}}
