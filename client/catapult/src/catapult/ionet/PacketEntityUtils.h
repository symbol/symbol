#pragma once
#include "Packet.h"
#include "PacketPayloadParser.h"
#include "catapult/model/EntityRange.h"

namespace catapult { namespace ionet {

	namespace detail {
		CATAPULT_INLINE
		size_t CalculatePacketDataSize(const Packet& packet) {
			constexpr auto Min_Size = sizeof(PacketHeader);
			if (packet.Size <= Min_Size) {
				if (packet.Size < Min_Size)
					CATAPULT_LOG(warning) << "packet size (" << packet.Size << ") must be at least " << Min_Size;

				return 0;
			}

			return packet.Size - Min_Size;
		}
	}

	/// Checks the real size of \a entity against its reported size and returns \c true if the sizes match.
	template<typename TEntity>
	bool IsSizeValid(const TEntity& entity) {
		return TEntity::CalculateRealSize(entity) == entity.Size;
	}

	/// Extracts entities from \a packet with a validity check (\a isValid).
	/// \note If the packet is invalid and/or contains partial entities, the returned range will be empty.
	template<typename TEntity, typename TIsValidPredicate>
	model::EntityRange<TEntity> ExtractEntitiesFromPacket(const Packet& packet, TIsValidPredicate isValid) {
		auto dataSize = detail::CalculatePacketDataSize(packet);
		auto offsets = ExtractEntityOffsets<TEntity>({ packet.Data(), dataSize }, isValid);
		return offsets.empty()
				? model::EntityRange<TEntity>()
				: model::EntityRange<TEntity>::CopyVariable(packet.Data(), dataSize, offsets);
	}

	/// Extracts a single entity from \a packet with a validity check (\a isValid).
	/// \note If the packet is invalid and/or contains partial or multiple entities, \c nullptr will be returned.
	template<typename TEntity, typename TIsValidPredicate>
	std::unique_ptr<TEntity> ExtractEntityFromPacket(const Packet& packet, TIsValidPredicate isValid) {
		auto dataSize = detail::CalculatePacketDataSize(packet);
		if (!ContainsSingleEntity<TEntity>({ packet.Data(), dataSize }, isValid))
			return nullptr;

		auto pEntity = utils::MakeUniqueWithSize<TEntity>(dataSize);
		std::memcpy(pEntity.get(), packet.Data(), dataSize);
		return pEntity;
	}

	/// Extracts fixed size structures from \a packet.
	/// \note If the packet is invalid and/or contains partial structures, the returned range will be empty.
	template<typename TStructure>
	model::EntityRange<TStructure> ExtractFixedSizeStructuresFromPacket(const Packet& packet) {
		auto dataSize = detail::CalculatePacketDataSize(packet);
		auto numStructures = CountFixedSizeStructures<TStructure>({ packet.Data(), dataSize });
		return 0 == numStructures
				? model::EntityRange<TStructure>()
				: model::EntityRange<TStructure>::CopyFixed(packet.Data(), numStructures);
	}
}}
