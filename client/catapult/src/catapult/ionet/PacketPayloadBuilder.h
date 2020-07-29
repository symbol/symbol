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
#include "PacketPayload.h"
#include "catapult/model/EntityRange.h"
#include "catapult/utils/IntegerMath.h"
#include "catapult/constants.h"

namespace catapult { namespace ionet {

	/// Packet payload builder for creating payloads composed of heterogeneous data.
	class PacketPayloadBuilder {
	public:
		/// Creates builder for a packet with the specified \a type.
		explicit PacketPayloadBuilder(PacketType type) : PacketPayloadBuilder(type, Default_Max_Packet_Data_Size)
		{}

		/// Creates builder for a packet with the specified \a type and max packet data size (\a maxPacketDataSize).
		PacketPayloadBuilder(PacketType type, uint32_t maxPacketDataSize)
				: m_maxPacketDataSize(maxPacketDataSize)
				, m_payload(type)
				, m_hasError(false)
		{}

	public:
		/// Appends a single entity (\a pEntity) to the payload.
		template<typename TEntity>
		bool appendEntity(const std::shared_ptr<TEntity>& pEntity) {
			if (!increaseSize(pEntity->Size))
				return false;

			m_payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(pEntity.get()), pEntity->Size });
			m_payload.m_entities.push_back(pEntity);
			return true;
		}

		/// Appends all \a entities to the payload.
		template<typename TEntity>
		bool appendEntities(const std::vector<std::shared_ptr<TEntity>>& entities) {
			for (const auto& pEntity : entities) {
				if (!appendEntity(pEntity))
					return false;
			}

			return true;
		}

		/// Appends all entities produced by \a generator to the payload.
		template<typename TEntityGenerator>
		bool appendGeneratedEntities(TEntityGenerator&& generator) {
			for (;;) {
				auto pEntity = generator();
				if (!pEntity)
					return true;

				if (!appendEntity(pEntity))
					return false;
			}
		}

		/// Appends a fixed size \a range to the payload.
		template<typename TEntity>
		bool appendRange(model::EntityRange<TEntity>&& range) {
			auto rangeSize = static_cast<uint32_t>(sizeof(TEntity) * range.size());
			if (!increaseSize(rangeSize))
				return false;

			if (!range.empty()) {
				m_payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(range.data()), rangeSize });
				m_payload.m_entities.push_back(std::make_shared<model::EntityRange<TEntity>>(std::move(range)));
			}

			return true;
		}

		/// Appends a fixed size \a value to the payload.
		template<typename TValue>
		bool appendValue(const TValue& value) {
			if (!increaseSize(sizeof(TValue)))
				return false;

			auto pValue = std::make_shared<TValue>(value);
			m_payload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(pValue.get()), sizeof(TValue) });
			m_payload.m_entities.push_back(pValue);
			return true;
		}

		/// Appends fixed size \a values to the payload.
		template<typename TValue>
		bool appendValues(const std::vector<TValue>& values) {
			auto valuesSize = static_cast<uint32_t>(sizeof(TValue) * values.size());
			if (!increaseSize(valuesSize))
				return false;

			if (!values.empty()) {
				auto pValues = utils::MakeSharedWithSize<uint8_t>(valuesSize);
				std::memcpy(pValues.get(), values.data(), valuesSize);
				m_payload.m_buffers.push_back({ pValues.get(), valuesSize });
				m_payload.m_entities.push_back(pValues);
			}

			return true;
		}

		/// Appends all values produced by \a generator to the payload.
		/// \note \a generator is expected to produce pointers to fixed-size data.
		template<typename TValueGenerator>
		bool appendGeneratedValues(TValueGenerator&& generator) {
			using ValueType = std::remove_const_t<std::remove_reference_t<decltype(*generator())>>;
			std::vector<ValueType> values;
			for (;;) {
				auto pValue = generator();
				if (!pValue)
					break;

				values.push_back(*pValue);
			}

			return appendValues(values);
		}

	public:
		/// Builds the packet payload.
		PacketPayload build() {
			return m_hasError ? PacketPayload() : std::move(m_payload);
		}

	private:
		bool increaseSize(uint32_t numBytes) {
			// check for overflow and against m_maxPacketDataSize
			auto& header = m_payload.m_header;
			if (m_hasError || !utils::CheckedAdd(header.Size, numBytes) || !IsPacketDataSizeValid(header, m_maxPacketDataSize)) {
				m_hasError = true;
				return false;
			}

			return true;
		}

	private:
		uint32_t m_maxPacketDataSize;
		PacketPayload m_payload;
		bool m_hasError;
	};
}}
