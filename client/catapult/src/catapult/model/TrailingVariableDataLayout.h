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
#include "SizeChecker.h"
#include "catapult/utils/NonCopyable.h"
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Defines a layout for a fixed header followed by variable data.
	template<typename TDerived, typename TVariableDataType>
	struct TrailingVariableDataLayout : public utils::NonCopyable {
	public:
		/// Data size.
		uint32_t Size;

	protected:
		/// Gets the start of the variable data part of \a derived.
		template<typename T>
		static auto PayloadStart(T& derived) {
			return !model::IsSizeValidT(derived) ? nullptr : ToBytePointer(derived) + sizeof(T);
		}

		/// Gets a typed pointer to the variable data \a pData.
		static const TVariableDataType* ToTypedPointer(const uint8_t* pData) {
			return reinterpret_cast<const TVariableDataType*>(pData);
		}

		/// Gets a typed pointer to the variable data \a pData.
		static TVariableDataType* ToTypedPointer(uint8_t* pData) {
			return reinterpret_cast<TVariableDataType*>(pData);
		}

	private:
		static const uint8_t* ToBytePointer(const TDerived& derived) {
			return reinterpret_cast<const uint8_t*>(&derived);
		}

		static uint8_t* ToBytePointer(TDerived& derived) {
			return reinterpret_cast<uint8_t*>(&derived);
		}
	};

#pragma pack(pop)

/// Defines \a NAME variable data accessors around a similarly named templated untyped data accessor.
/// \a SIZE_POSTFIX specifies the postfix of the corresponding size or count field.
#define DEFINE_TRAILING_VARIABLE_DATA_LAYOUT_ACCESSORS(NAME, SIZE_POSTFIX) \
	/* Returns a const pointer to the typed data contained in this entity. */ \
	const auto* NAME##Ptr() const { \
		return NAME##SIZE_POSTFIX ? ToTypedPointer(PayloadStart(*this)) : nullptr; \
	} \
	\
	/* Returns a pointer to the typed data contained in this entity. */ \
	auto* NAME##Ptr() { \
		return NAME##SIZE_POSTFIX ? ToTypedPointer(PayloadStart(*this)) : nullptr; \
	}
}}
