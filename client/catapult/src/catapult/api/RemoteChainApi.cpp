#include "RemoteChainApi.h"
#include "ChainPackets.h"
#include "RemoteApiUtils.h"
#include "RemoteRequestDispatcher.h"
#include "catapult/ionet/PacketEntityUtils.h"

namespace catapult { namespace api {

	namespace {
		// region traits

		struct ChainInfoTraits {
		public:
			using ResultType = ChainInfo;
			static constexpr auto PacketType() { return ionet::PacketType::Chain_Info; }
			static constexpr auto FriendlyName() { return "chain info"; }

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(PacketType());
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				const auto* pResponse = ionet::CoercePacket<ChainInfoResponse>(&packet);
				if (!pResponse)
					return false;

				result.Height = pResponse->Height;
				result.Score = model::ChainScore(pResponse->ScoreHigh, pResponse->ScoreLow);
				return true;
			}
		};

		struct HashesFromTraits {
		public:
			using ResultType = model::HashRange;
			static constexpr auto PacketType() { return ionet::PacketType::Block_Hashes; }
			static constexpr auto FriendlyName() { return "hashes from"; }

			static auto CreateRequestPacketPayload(Height height) {
				auto pPacket = ionet::CreateSharedPacket<BlockHashesRequest>();
				pPacket->Height = height;
				return ionet::PacketPayload(pPacket);
			}

		public:
			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractFixedSizeStructuresFromPacket<Hash256>(packet);
				return !result.empty();
			}
		};

		struct BlockAtTraits : public RegistryDependentTraits<model::Block> {
		public:
			using ResultType = std::shared_ptr<const model::Block>;
			static constexpr auto PacketType() { return ionet::PacketType::Pull_Block; }
			static constexpr auto FriendlyName() { return "block at"; }

			static auto CreateRequestPacketPayload(Height height) {
				auto pPacket = ionet::CreateSharedPacket<PullBlockRequest>();
				pPacket->Height = height;
				return ionet::PacketPayload(pPacket);
			}

		public:
			using RegistryDependentTraits::RegistryDependentTraits;

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntityFromPacket<model::Block>(packet, *this);
				return !!result;
			}
		};

		struct BlocksFromTraits : public RegistryDependentTraits<model::Block> {
		public:
			using ResultType = model::BlockRange;
			static constexpr auto PacketType() { return ionet::PacketType::Pull_Blocks; }
			static constexpr auto FriendlyName() { return "blocks from"; }

			static auto CreateRequestPacketPayload(Height height, const BlocksFromOptions& options) {
				auto pPacket = ionet::CreateSharedPacket<PullBlocksRequest>();
				pPacket->Height = height;
				pPacket->NumBlocks = options.NumBlocks;
				pPacket->NumResponseBytes = options.NumBytes;
				return ionet::PacketPayload(pPacket);
			}

		public:
			using RegistryDependentTraits::RegistryDependentTraits;

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				result = ionet::ExtractEntitiesFromPacket<model::Block>(packet, *this);
				return !result.empty() || sizeof(ionet::PacketHeader) == packet.Size;
			}
		};

		// endregion

		class DefaultRemoteChainApi : public RemoteChainApi {
		private:
			template<typename TTraits>
			using FutureType = thread::future<typename TTraits::ResultType>;

		public:
			explicit DefaultRemoteChainApi(ionet::PacketIo& io, const model::TransactionRegistry* pRegistry)
					: m_pRegistry(pRegistry)
					, m_impl(io)
			{}

		public:
			FutureType<ChainInfoTraits> chainInfo() const override {
				return m_impl.dispatch(ChainInfoTraits());
			}

			FutureType<HashesFromTraits> hashesFrom(Height height) const override {
				return m_impl.dispatch(HashesFromTraits(), height);
			}

			FutureType<BlockAtTraits> blockLast() const override {
				return m_impl.dispatch(BlockAtTraits(*m_pRegistry), Height(0));
			}

			FutureType<BlockAtTraits> blockAt(Height height) const override {
				return m_impl.dispatch(BlockAtTraits(*m_pRegistry), height);
			}

			FutureType<BlocksFromTraits> blocksFrom(Height height, const BlocksFromOptions& options) const override {
				return m_impl.dispatch(BlocksFromTraits(*m_pRegistry), height, options);
			}

		private:
			const model::TransactionRegistry* m_pRegistry;
			mutable RemoteRequestDispatcher m_impl;
		};
	}

	std::unique_ptr<ChainApi> CreateRemoteChainApiWithoutRegistry(ionet::PacketIo& io) {
		// since the returned interface is only chain-api, the registry is unused and can be null
		return std::make_unique<DefaultRemoteChainApi>(io, nullptr);
	}

	std::unique_ptr<RemoteChainApi> CreateRemoteChainApi(ionet::PacketIo& io, const model::TransactionRegistry& registry) {
		return std::make_unique<DefaultRemoteChainApi>(io, &registry);
	}
}}
