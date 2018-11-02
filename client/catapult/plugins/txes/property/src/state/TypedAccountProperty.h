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
#include "AccountProperty.h"
#include "PropertyUtils.h"

namespace catapult { namespace state {

	/// Typed account property.
	template<typename TPropertyValue>
	class TypedAccountProperty {
	public:
		/// Creates a typed account property around \a property.
		explicit TypedAccountProperty(const AccountProperty& property) : m_property(property)
		{}

	public:
		/// Gets the property descriptor of the underlying property.
		const PropertyDescriptor& descriptor() const {
			return m_property.descriptor();
		}

		/// Gets the number of values of the underlying property.
		size_t size() const {
			return m_property.values().size();
		}

		/// Returns \c true if the underlying property contains \a value.
		bool contains(const TPropertyValue& value) const {
			return m_property.contains(ToVector(value));
		}

	public:
		/// Returns \c true if \a modification can be applied to the underlying property.
		bool canAllow(const model::PropertyModification<TPropertyValue>& modification) const {
			return m_property.canAllow({ modification.ModificationType, ToVector(modification.Value) });
		}

		/// Returns \c true if \a modification can be applied to the underlying property.
		bool canBlock(const model::PropertyModification<TPropertyValue>& modification) const {
			return m_property.canBlock({ modification.ModificationType, ToVector(modification.Value) });
		}

	private:
		const AccountProperty& m_property;
	};
}}
