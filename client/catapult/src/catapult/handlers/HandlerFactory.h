#pragma once
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/model/EntityRange.h"

namespace catapult { namespace handlers {

	/// Factory for creating a handler that returns batch of entities.
	template<typename TRequestTraits>
	struct BatchHandlerFactory {
	private:
		using RequestStructureType = typename TRequestTraits::RequestStructureType;
		using RequestRangeType = model::EntityRange<RequestStructureType>;

	public:
		/// The packet type supported by the created handler.
		static constexpr auto Packet_Type = TRequestTraits::Packet_Type;

	public:
		/// Creates a handler that uses \a resultsSupplier to supply results.
		template<typename TResultsSupplier>
		static auto Create(TResultsSupplier resultsSupplier) {
			return [resultsSupplier](const auto& packet, auto& context) {
				auto info = BatchHandlerFactory::ProcessRequest(packet);
				if (!info.IsValid)
					return;

				auto results = resultsSupplier(info.Range);
				context.response(TRequestTraits::ToPayload(results));
			};
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
			if (TRequestTraits::Packet_Type != packet.Type)
				return PacketInfo<RequestRangeType>(RequestRangeType());

			return PacketInfo<RequestRangeType>(ionet::ExtractFixedSizeStructuresFromPacket<RequestStructureType>(packet));
		}
	};
}}
