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
		static PacketPayload FromEntities(PacketType type, const std::vector<std::shared_ptr<TEntity>>& entities);

		/// Creates a packet payload with the specified packet \a type around a fixed size structure \a range.
		template<typename TStructure>
		static PacketPayload FromFixedSizeRange(PacketType type, model::EntityRange<TStructure>&& range);

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

	private:
		friend class PacketPayloadBuilder;
	};

	/// A packet payload builder for creating payloads composed of heterogeneous data.
	class PacketPayloadBuilder {
	public:
		/// Creates builder for a packet with the specified \a type.
		explicit PacketPayloadBuilder(PacketType type) : m_payload(type)
		{}

	public:
		/// Appends all \a entities to the payload.
		template<typename TEntity>
		void appendEntities(const std::vector<std::shared_ptr<TEntity>>& entities) {
			for (const auto& pEntity : entities)
				appendEntity(pEntity);
		}

		/// Appends a single entity (\a pEntity) to the payload
		template<typename TEntity>
		void appendEntity(const std::shared_ptr<TEntity>& pEntity) {
			increaseSize(pEntity->Size);
			m_payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(pEntity.get()), pEntity->Size });
			m_payload.m_entities.push_back(pEntity);
		}

		/// Appends a fixed size \a range to the payload.
		template<typename TEntity>
		void appendRange(model::EntityRange<TEntity>&& range) {
			if (range.empty())
				return;

			auto rangeSize = static_cast<uint32_t>(sizeof(TEntity) * range.size());
			increaseSize(rangeSize);
			m_payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(range.data()), rangeSize });
			m_payload.m_entities.push_back(std::make_shared<model::EntityRange<TEntity>>(std::move(range)));
		}

		/// Appends a fixed size \a value to the payload.
		template<typename TValue>
		void appendValue(const TValue& value) {
			increaseSize(sizeof(TValue));

			auto pValue = std::make_shared<TValue>(value);
			m_payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(pValue.get()), sizeof(TValue) });
			m_payload.m_entities.push_back(pValue);
		}

	public:
		/// Builds the packet payload.
		PacketPayload build() {
			return std::move(m_payload);
		}

	private:
		void increaseSize(uint32_t numBytes) {
			// check for overflow
			auto& header = m_payload.m_header;
			if (header.Size > header.Size + numBytes)
				CATAPULT_THROW_RUNTIME_ERROR("cannot append data that will exceed max packet size");

			header.Size += numBytes;
		}

	private:
		PacketPayload m_payload;
	};

	// out of line to use PacketPayloadBuilder

	template<typename TEntity>
	PacketPayload PacketPayload::FromEntities(PacketType type, const std::vector<std::shared_ptr<TEntity>>& entities) {
		PacketPayloadBuilder builder(type);
		builder.appendEntities(entities);
		return builder.build();
	}

	template<typename TStructure>
	PacketPayload PacketPayload::FromFixedSizeRange(PacketType type, model::EntityRange<TStructure>&& range) {
		PacketPayloadBuilder builder(type);
		builder.appendRange(std::move(range));
		return builder.build();
	}
}}
