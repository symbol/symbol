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
#include "PluginManager.h"
#include "catapult/handlers/CacheEntryInfosProducerFactory.h"
#include "catapult/handlers/HandlerFactory.h"
#include "catapult/handlers/StatePathHandlerFactory.h"

namespace catapult { namespace plugins {

	/// Utility class for registering cache-dependent handlers.
	template<typename TCacheDescriptor>
	class CacheHandlers {
	public:
		/// Registers all cache handlers in \a pluginManager for cache with specified facility code.
		template<model::FacilityCode FacilityCode>
		static void Register(PluginManager& pluginManager) {
			using KeyType = typename TCacheDescriptor::KeyType;
			using CacheType = typename TCacheDescriptor::CacheType;
			using CachePacketTypes = CachePacketTypesT<FacilityCode>;

			pluginManager.addHandlerHook([](auto& handlers, const cache::CatapultCache& cache) {
				using PacketType = StatePathRequestPacket<CachePacketTypes::State_Path, KeyType>;
				handlers::RegisterStatePathHandler<PacketType>(handlers, cache.sub<CacheType>());
			});

			pluginManager.addDiagnosticHandlerHook([](auto& handlers, const cache::CatapultCache& cache) {
				using RequestTraits = BatchHandlerFactoryTraits<CachePacketTypes::Diagnostic_Infos, KeyType>;
				handlers::BatchHandlerFactory<RequestTraits>::RegisterOne(
						handlers,
						handlers::CacheEntryInfosProducerFactory<TCacheDescriptor>::Create(cache.sub<CacheType>()));
			});
		}

	private:
		template<model::FacilityCode FacilityCode>
		struct CachePacketTypesT {
			static constexpr auto State_Path = static_cast<ionet::PacketType>(0x200 + utils::to_underlying_type(FacilityCode));
			static constexpr auto Diagnostic_Infos = static_cast<ionet::PacketType>(0x400 + utils::to_underlying_type(FacilityCode));
		};

		template<ionet::PacketType PacketType, typename TCacheKey>
		struct StatePathRequestPacket : public ionet::Packet {
			static constexpr ionet::PacketType Packet_Type = PacketType;

			TCacheKey Key;
		};

		template<ionet::PacketType PacketType, typename TCacheKey>
		struct BatchHandlerFactoryTraits {
			static constexpr ionet::PacketType Packet_Type = PacketType;

			using RequestStructureType = TCacheKey;
		};
	};
}}
