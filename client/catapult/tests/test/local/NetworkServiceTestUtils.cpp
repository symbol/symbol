#include "NetworkServiceTestUtils.h"
#include "catapult/ionet/Packet.h"
#include "tests/test/core/BlockTestUtils.h"

namespace catapult { namespace test {

	namespace {
		template<typename TEntity>
		std::shared_ptr<ionet::Packet> GeneratePushEntityPacket(ionet::PacketType type, const TEntity& entity) {
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(entity.Size);
			pPacket->Type = type;

			std::memcpy(pPacket.get() + 1, &entity, entity.Size);
			return pPacket;
		}
	}

	std::shared_ptr<ionet::Packet> GenerateRandomBlockPacket() {
		return GeneratePushEntityPacket(ionet::PacketType::Push_Block, *GenerateEmptyRandomBlock());
	}

	std::shared_ptr<ionet::Packet> GenerateRandomTransactionPacket() {
		return GeneratePushEntityPacket(ionet::PacketType::Push_Transactions, *GenerateRandomTransaction());
	}
}}
