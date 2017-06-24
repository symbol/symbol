#pragma once
#include "Packet.h"
#include "catapult/model/EntityRange.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

namespace catapult { namespace ionet {

	/// A packet payload that can be written.
	class PacketPayload {
	public:
		/// Creates a default (empty) packet payload.
		PacketPayload() {
			m_header.Size = 0u;
			m_header.Type = PacketType::Undefined;
		}

		/// Creates a data-less packet payload with the specified \a type.
		explicit PacketPayload(PacketType type) {
			m_header.Size = sizeof(PacketHeader);
			m_header.Type = type;
		}

		/// Creates a packet payload around a single shared packet (\a pPacket).
		/// \note Implicit conversions from std::shared_ptr<Packet> to PacketPayload should be allowed.
		template<typename TPacket>
		PacketPayload(const std::shared_ptr<TPacket>& pPacket) : m_pPacket(pPacket) {
			if (pPacket->Size < sizeof(PacketHeader))
				CATAPULT_THROW_INVALID_ARGUMENT("cannot create payload around packet with invalid size");

			m_header.Size = pPacket->Size;
			m_header.Type = pPacket->Type;

			if (pPacket->Size == sizeof(PacketHeader))
				return;

			m_buffers.push_back({ pPacket->Data(), m_header.Size - sizeof(PacketHeader) });
		}

		/// Creates a packet payload around a single unique packet (\a pPacket).
		/// \note Implicit conversions from std::unique_ptr<Packet> to PacketPayload should be allowed.
		template<typename TPacket>
		PacketPayload(std::unique_ptr<TPacket>&& pPacket) : PacketPayload(std::shared_ptr<TPacket>(std::move(pPacket)))
		{}

	public:
		/// Creates a packet payload with the specified packet \a type around an entity (\a pEntity).
		template<typename TEntity>
		static PacketPayload FromEntity(PacketType type, const std::shared_ptr<TEntity>& pEntity) {
			return FromEntities<TEntity>(type, { pEntity });
		}

		/// Creates a packet payload with the specified packet \a type around multiple \a entities.
		template<typename TEntity>
		static PacketPayload FromEntities(PacketType type, const std::vector<std::shared_ptr<TEntity>>& entities) {
			PacketPayload payload;
			payload.m_header.Size = sizeof(PacketHeader);
			payload.m_header.Type = type;

			for (auto pEntity : entities) {
				// check for overflow
				if (payload.m_header.Size > payload.m_header.Size + pEntity->Size)
					CATAPULT_THROW_RUNTIME_ERROR("entities are too large for single packet payload");

				payload.m_header.Size += pEntity->Size;
				payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(pEntity.get()), pEntity->Size });
				payload.m_entities.push_back(pEntity);
			}

			return payload;
		}

		/// Creates a packet payload with the specified packet \a type around a fixed size entity \a range.
		template<typename TEntity>
		static PacketPayload FromFixedSizeRange(PacketType type, model::EntityRange<TEntity>&& range) {
			auto rangeSize = static_cast<uint32_t>(sizeof(TEntity) * range.size());
			PacketPayload payload;
			payload.m_header.Size = sizeof(PacketHeader) + rangeSize;
			payload.m_header.Type = type;

			if (!range.empty()) {
				payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(&*range.cbegin()), rangeSize });
				payload.m_entities.push_back(std::make_shared<model::EntityRange<TEntity>>(std::move(range)));
			}

			return payload;
		}

	public:
		/// Returns \c true if this packet payload is unset.
		bool unset() const {
			return 0u == m_header.Size;
		}

		/// The packet header.
		const PacketHeader& header() const {
			return m_header;
		}

		/// The packet data.
		const std::vector<RawBuffer>& buffers() const {
			return m_buffers;
		}

	private:
		PacketHeader m_header;
		std::vector<RawBuffer> m_buffers;

		// the backing data
		std::shared_ptr<Packet> m_pPacket;
		std::vector<std::shared_ptr<const void>> m_entities;
	};
}}
