/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/ionet/PacketPayloadBuilder.h"
#include "catapult/model/EntityRange.h"

namespace catapult { namespace handlers {

	/// Factory for creating a handler that returns batch of entities.
	template<typename TRequestTraits>
	class BatchHandlerFactory {
	public:
		/// Packet type supported by the created handler.
		static constexpr auto Packet_Type = TRequestTraits::Packet_Type;

	public:
		/// Registers a handler in \a handlers that uses \a resultsProducerFactory to produce results.
		/// \note producer does not accept any arguments.
		template<typename TResultsProducerFactory>
		static void RegisterZero(ionet::ServerPacketHandlers& handlers, TResultsProducerFactory resultsProducerFactory) {
			auto maxPacketDataSize = handlers.maxPacketDataSize();
			handlers.registerHandler(Packet_Type, [resultsProducerFactory, maxPacketDataSize](const auto& packet, auto& handlerContext) {
				if (!IsPacketValid(packet, Packet_Type))
					return;

				// always send a response (even if empty) in order to always acknowledge the request
				SetResponse(handlerContext, maxPacketDataSize, resultsProducerFactory());
			});
		}

		/// Registers a handler in \a handlers that uses \a resultsProducerFactory to produce results.
		/// \note producer accepts a single range argument.
		template<typename TResultsProducerFactory>
		static void RegisterOne(ionet::ServerPacketHandlers& handlers, TResultsProducerFactory resultsProducerFactory) {
			auto maxPacketDataSize = handlers.maxPacketDataSize();
			handlers.registerHandler(Packet_Type, [resultsProducerFactory, maxPacketDataSize](const auto& packet, auto& handlerContext) {
				auto info = BatchHandlerFactory::ProcessRequest(packet);
				if (!info.IsValid)
					return;

				// always send a response (even if empty) in order to always acknowledge the request
				SetResponse(handlerContext, maxPacketDataSize, resultsProducerFactory(info.Range));
			});
		}

	private:
		template<typename TProducer>
		static void SetResponse(ionet::ServerPacketHandlerContext& handlerContext, uint32_t maxPacketDataSize, TProducer&& producer) {
			auto builder = ionet::PacketPayloadBuilder(Packet_Type, maxPacketDataSize);
			Append(AppendAccessor<TRequestTraits>(), builder, producer);
			handlerContext.response(builder.build());
		}

	private:
		enum class AppendType { Entities, Values };
		using EntitiesAppendFlag = std::integral_constant<AppendType, AppendType::Entities>;
		using ValuesAppendFlag = std::integral_constant<AppendType, AppendType::Values>;

		template<typename T, typename = void>
		struct AppendAccessor : EntitiesAppendFlag {};

		template<typename T>
		struct AppendAccessor<T, utils::traits::is_type_expression_t<decltype(T::Should_Append_As_Values)>> : ValuesAppendFlag {};

	private:
		template<typename TProducer>
		static void Append(EntitiesAppendFlag, ionet::PacketPayloadBuilder& builder, TProducer& producer) {
			builder.appendGeneratedEntities(producer);
		}

		template<typename TProducer>
		static void Append(ValuesAppendFlag, ionet::PacketPayloadBuilder& builder, TProducer& producer) {
			builder.appendGeneratedValues(producer);
		}

	private:
		template<typename TRange>
		struct PacketInfo {
		public:
			explicit PacketInfo(TRange&& range)
					: Range(std::move(range))
					, IsValid(!Range.empty())
			{}

		public:
			TRange Range;
			bool IsValid;
		};

	private:
		static auto ProcessRequest(const ionet::Packet& packet) {
			using RequestStructureType = typename TRequestTraits::RequestStructureType;
			using RequestRangeType = model::EntityRange<RequestStructureType>;

			return TRequestTraits::Packet_Type != packet.Type
					? PacketInfo<RequestRangeType>(RequestRangeType())
					: PacketInfo<RequestRangeType>(ionet::ExtractFixedSizeStructuresFromPacket<RequestStructureType>(packet));
		}
	};
}}
