#pragma once
#include "catapult/utils/NonCopyable.h"
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a size prefixed entity.
	struct SizePrefixedEntity : public utils::NonCopyable {
	public:
		/// The entity size.
		uint32_t Size;

	protected:
		/// Returns byte-based const pointer to this entity.
		const uint8_t* ToBytePointer() const;

		/// Returns byte-based pointer to this entity.
		uint8_t* ToBytePointer();

		/// Gets the start of the variable data part of \a entity.
		template<typename T>
		static auto PayloadStart(T& entity) {
			return entity.Size != T::CalculateRealSize(entity) ? nullptr : entity.ToBytePointer() + sizeof(T);
		}

	public:
		/// Returns \c true if this entity is equal to \a rhs.
		bool operator==(const SizePrefixedEntity& rhs) const;

		/// Returns \c true if this entity is not equal to \a rhs.
		bool operator!=(const SizePrefixedEntity& rhs) const;
	};

#pragma pack(pop)
}}
