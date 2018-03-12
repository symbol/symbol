#pragma once
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Defines a layout for a fixed header followed by variable data.
	/// \note Do not derive from SizePrefixedEntity in order to allow copying.
	template<typename TDerived, typename TVariableDataType>
	struct TrailingVariableDataLayout {
	public:
		/// The size.
		uint32_t Size;

	protected:
		/// Gets the start of the variable data part of \a derived.
		template<typename T>
		static auto PayloadStart(T& derived) {
			return derived.Size != TDerived::CalculateRealSize(derived) ? nullptr : ToBytePointer(derived) + sizeof(T);
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
}}
