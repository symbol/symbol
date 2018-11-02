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
#include "src/model/PropertyTypes.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace state {

	/// Operation type.
	enum class OperationType {
		/// Property contains allowed values.
		Allow,

		/// Property contains blocked value.
		Block
	};

	/// Property descriptor.
	class PropertyDescriptor {
	public:
		/// Creates a property descriptor around \a propertyType.
		constexpr explicit PropertyDescriptor(model::PropertyType propertyType) : m_propertyType(propertyType)
		{}

	public:
		/// Gets the value specific part of the property type.
		constexpr model::PropertyType propertyType() const {
			return StripFlag(m_propertyType, model::PropertyType::Block);
		}

		/// Gets the operation type.
		constexpr OperationType operationType() const {
			return model::HasFlag(model::PropertyType::Block, m_propertyType) ? OperationType::Block : OperationType::Allow;
		}

		/// Gets the raw property type.
		constexpr model::PropertyType raw() const {
			return m_propertyType;
		}

	private:
		static constexpr model::PropertyType StripFlag(model::PropertyType lhs, model::PropertyType flag) {
			return static_cast<model::PropertyType>(utils::to_underlying_type(lhs) & ~utils::to_underlying_type(flag));
		}

	private:
		model::PropertyType m_propertyType;
	};
}}
