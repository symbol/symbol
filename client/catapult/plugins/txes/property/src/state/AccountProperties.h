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
#include "TypedAccountProperty.h"

namespace catapult { namespace state {

	/// Account properties.
	class AccountProperties {
	private:
		using PropertiesMap = std::map<model::PropertyType, AccountProperty>;
		using const_iterator = PropertiesMap::const_iterator;

	public:
		/// Creates account properties around \a address.
		explicit AccountProperties(const Address& address);

	public:
		/// Gets the account address.
		const Address& address() const;

		/// Gets the number of account properties.
		size_t size() const;

		/// Returns \c true if no property has any value.
		bool isEmpty() const;

	public:
		/// Gets the const typed account property specified by \a propertyType.
		template<typename TPropertyValue>
		TypedAccountProperty<TPropertyValue> property(model::PropertyType propertyType) const {
			const auto& property = getProperty<const PropertiesMap, const AccountProperty>(m_properties, propertyType);
			return TypedAccountProperty<TPropertyValue>(property);
		}

		/// Gets the const account property specified by \a propertyType.
		const AccountProperty& property(model::PropertyType propertyType) const;

		/// Gets the account property specified by \a propertyType.
		AccountProperty& property(model::PropertyType propertyType);

	public:
		/// Returns a const iterator to the first account property.
		const_iterator begin() const;

		/// Returns a const iterator to the element following the last account property.
		const_iterator end() const;

	private:
		void addProperty(model::PropertyType propertyType, size_t propertyValueSize);

	private:
		template<typename TPropertiesMap, typename TProperty>
		static TProperty& getProperty(TPropertiesMap& propertiesMap, model::PropertyType propertyType) {
			auto iter = propertiesMap.find(PropertyDescriptor(propertyType).propertyType());
			if (propertiesMap.cend() == iter)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown property type", static_cast<uint16_t>(propertyType));

			return iter->second;
		}

	private:
		Address m_address;
		PropertiesMap m_properties;
	};
}}
