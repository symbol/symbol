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

#include "AccountProperties.h"
#include "catapult/model/EntityType.h"

namespace catapult { namespace state {

	AccountProperties::AccountProperties(const Address& address) : m_address(address) {
		addProperty(model::PropertyType::Address, Address_Decoded_Size);
		addProperty(model::PropertyType::MosaicId, sizeof(MosaicId));
		addProperty(model::PropertyType::TransactionType, sizeof(model::EntityType));
	}

	const Address& AccountProperties::address() const {
		return m_address;
	}

	size_t AccountProperties::size() const {
		return m_properties.size();
	}

	bool AccountProperties::isEmpty() const {
		return std::all_of(begin(), end(), [](const auto& pair) {
			return pair.second.values().empty();
		});
	}

	const AccountProperty& AccountProperties::property(model::PropertyType propertyType) const {
		return getProperty<const PropertiesMap, const AccountProperty>(m_properties, propertyType);
	}

	AccountProperty& AccountProperties::property(model::PropertyType propertyType) {
		return getProperty<PropertiesMap, AccountProperty>(m_properties, propertyType);
	}

	AccountProperties::const_iterator AccountProperties::begin() const {
		return m_properties.cbegin();
	}

	AccountProperties::const_iterator AccountProperties::end() const {
		return m_properties.cend();
	}

	void AccountProperties::addProperty(model::PropertyType propertyType, size_t propertyValueSize) {
		m_properties.emplace(propertyType, AccountProperty(propertyType, propertyValueSize));
	}
}}
