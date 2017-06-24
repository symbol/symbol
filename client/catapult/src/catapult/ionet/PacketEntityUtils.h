#pragma once
#include "Packet.h"
#include "catapult/model/EntityRange.h"
#include "catapult/utils/Logging.h"
#include <memory>

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

		template<typename TEntity, typename TIsValidPredicate>
		bool IsEntityValid(const TEntity& entity, TIsValidPredicate isValid) {
			if (isValid(entity))
				return true;

			CATAPULT_LOG(warning) << "entity (header size = " << sizeof(TEntity)
					<< ") has failed validity check with size " << entity.Size;
			return false;
		}
	}

	/// Extracts entities from \a packet with a validity check (\a isValid).
	/// \note If the packet is invalid and/or contains partial entities, the returned range will be empty.
	template<typename TEntity, typename TIsValidPredicate>
	model::EntityRange<TEntity> ExtractEntitiesFromPacket(const Packet& packet, TIsValidPredicate isValid) {
		auto dataSize = detail::CalculatePacketDataSize(packet);
		if (0 == dataSize)
			return model::EntityRange<TEntity>();

#define LOG_EXCEEDS_REMAINING_BYTES(SIZE, MESSAGE) \
	CATAPULT_LOG(warning) << MESSAGE << " (" << SIZE << ") exceeds remaining bytes (" << remainingBytes << ")";

		std::vector<size_t> offsets;
		auto remainingBytes = dataSize;
		while (0 != remainingBytes) {
			if (sizeof(TEntity) > remainingBytes) {
				LOG_EXCEEDS_REMAINING_BYTES(sizeof(TEntity), "entity header size");
				return model::EntityRange<TEntity>();
			}

			auto offset = dataSize - remainingBytes;
			const auto& entity = reinterpret_cast<const TEntity&>(*(packet.Data() + offset));
			if (!detail::IsEntityValid(entity, isValid))
				return model::EntityRange<TEntity>();

			if (entity.Size > remainingBytes) {
				LOG_EXCEEDS_REMAINING_BYTES(entity.Size, "entity size");
				return model::EntityRange<TEntity>();
			}

			offsets.push_back(offset);
			remainingBytes -= entity.Size;
		}

#undef LOG_EXCEEDS_REMAINING_BYTES

		return model::EntityRange<TEntity>::CopyVariable(packet.Data(), dataSize, offsets);
	}

#define LOG_DATA_SIZE_ERROR(SIZE, MESSAGE) \
	CATAPULT_LOG(warning) << "packet data size (" << dataSize << ") " << MESSAGE << " (" << SIZE << ")";

	/// Extracts a single entity from \a packet with a validity check (\a isValid).
	/// \note If the packet is invalid and/or contains partial or multiple entities, \c nullptr will be returned.
	template<typename TEntity, typename TIsValidPredicate>
	std::unique_ptr<TEntity> ExtractEntityFromPacket(const Packet& packet, TIsValidPredicate isValid) {
		auto dataSize = detail::CalculatePacketDataSize(packet);
		if (0 == dataSize)
			return nullptr;

		if (dataSize < sizeof(TEntity)) {
			LOG_DATA_SIZE_ERROR(sizeof(TEntity), "must be at least entity size");
			return nullptr;
		}

		const auto& entity = reinterpret_cast<const TEntity&>(*(packet.Data()));
		if (!detail::IsEntityValid(entity, isValid))
			return nullptr;

		if (entity.Size != dataSize) {
			LOG_DATA_SIZE_ERROR(entity.Size, "is inconsistent with entity size");
			return nullptr;
		}

		std::unique_ptr<TEntity> pEntity(reinterpret_cast<TEntity*>(::operator new (dataSize)));
		std::memcpy(pEntity.get(), packet.Data(), dataSize);
		return pEntity;
	}

	/// Extracts fixed size entities from \a packet.
	/// \note If the packet is invalid and/or contains partial entities, the returned range will be empty.
	template<typename TEntity>
	model::EntityRange<TEntity> ExtractFixedSizeEntitiesFromPacket(const Packet& packet) {
		auto dataSize = detail::CalculatePacketDataSize(packet);
		if (0 == dataSize)
			return model::EntityRange<TEntity>();

		constexpr auto Entity_Size = sizeof(TEntity);
		if (0 != dataSize % Entity_Size) {
			LOG_DATA_SIZE_ERROR(Entity_Size, "contains fractional entities of size");
			return model::EntityRange<TEntity>();
		}

		auto numEntities = dataSize / Entity_Size;
		return model::EntityRange<TEntity>::CopyFixed(packet.Data(), numEntities);
	}

#undef LOG_DATA_SIZE_ERROR
}}
