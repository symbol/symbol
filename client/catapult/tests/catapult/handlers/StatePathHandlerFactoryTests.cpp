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

#include "catapult/handlers/StatePathHandlerFactory.h"
#include "catapult/cache/SynchronizedCache.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include "tests/test/plugins/BasicBatchHandlerTests.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace handlers {

#define TEST_CLASS StatePathHandlerFactoryTests

	namespace {
		constexpr auto Mock_Packet_Type = static_cast<ionet::PacketType>(0x1234);
		using TestPayloadType = uint64_t;
		constexpr auto Payload_Size = sizeof(TestPayloadType);

		// region helpers

		auto CreateRandomLeafNode() {
			return tree::LeafTreeNode(tree::TreeNodePath(test::Random()), test::GenerateRandomByteArray<Hash256>());
		}

		auto CreateRandomBranchNode() {
			auto node = tree::BranchTreeNode(tree::TreeNodePath(test::Random()));
			for (auto i = 0u; i < 16u; ++i)
				node.setLink(test::GenerateRandomByteArray<Hash256>(), i);

			return node;
		}

		auto SerializePath(const std::vector<tree::TreeNode>& path) {
			std::vector<uint8_t> serializedPath;
			for (const auto& node : path) {
				auto serializedNode = tree::PatriciaTreeSerializer::SerializeValue(node);
				const auto* pData = reinterpret_cast<const uint8_t*>(serializedNode.data());
				serializedPath.insert(serializedPath.end(), pData, pData + serializedNode.size());
			}

			return serializedPath;
		}

		// endregion

		// region cache

		using StatePath = std::vector<tree::TreeNode>;

		class MockCacheView {
		public:
			MockCacheView(bool result, const StatePath& path)
					: m_result(result)
					, m_path(path)
			{}

		public:
			auto tryLookup(const uint64_t&, StatePath& path) const {
				for (const auto& node : m_path)
					path.push_back(node.copy());

				return std::make_pair(Hash256(), m_result);
			}

		private:
			const bool m_result;
			const StatePath& m_path;
		};

		class MockCache {
		public:
			MockCache() : m_lookupResult(false)
			{}

		public:
			auto createView() const {
				auto readLock = m_lock.acquireReader();
				return cache::LockedCacheView<MockCacheView>(MockCacheView(m_lookupResult, m_path), std::move(readLock));
			}

		public:
			auto setLookupResult(bool result, size_t numElements) {
				m_lookupResult = result;

				for (auto i = 0u; i < numElements; ++i)
					m_path.push_back(i % 2 ? tree::TreeNode(CreateRandomBranchNode()) : tree::TreeNode(CreateRandomLeafNode()));

				return SerializePath(m_path);
			}

		private:
			mutable utils::SpinReaderWriterLock m_lock;
			bool m_lookupResult;
			StatePath m_path;
		};

		// endregion

		// region test traits

		struct StatePathRequestPacket : public ionet::Packet {
			static constexpr ionet::PacketType Packet_Type = Mock_Packet_Type;

			TestPayloadType Key;
		};

		struct StatePathHandlerFactoryTraits {
		public:
			static constexpr auto Packet_Type = Mock_Packet_Type;
			static constexpr auto Valid_Request_Payload_Size = Payload_Size;

			using RequestPayloadType = TestPayloadType;

		public:
			class TestContext {
			public:
				const auto& getCache() const {
					return m_cache;
				}

				auto& getCache() {
					return m_cache;
				}

			public:
				void assertRejected() const {
					// nothing to check
				}

			private:
				MockCache m_cache;
			};

		public:
			static void RegisterHandler(ionet::ServerPacketHandlers& handlers, const MockCache& cache) {
				RegisterStatePathHandler<StatePathRequestPacket>(handlers, cache);
			}
		};

		// endregion

		// region base tests

		template<typename TTraits>
		struct CacheHandlerTraits {
			using HandlerContext = typename TTraits::TestContext;

			static void RegisterHandler(ionet::ServerPacketHandlers& handlers, const HandlerContext& registerHandlerContext) {
				TTraits::RegisterHandler(handlers, registerHandlerContext.getCache());
			}
		};

		using BasicHandlerTests = test::BasicBatchHandlerTests<
			StatePathHandlerFactoryTraits,
			CacheHandlerTraits<StatePathHandlerFactoryTraits>>;

		// endregion

		// region valid packet tests

		template<typename TArrange, typename TAssertResponse>
		void AssertPacketIsAccepted(TArrange arrange, TAssertResponse assertResponse) {
			// Arrange:
			StatePathHandlerFactoryTraits::TestContext testContext;
			ionet::ServerPacketHandlers handlers;
			StatePathHandlerFactoryTraits::RegisterHandler(handlers, testContext.getCache());
			auto pPacket = test::CreateRandomPacket(Payload_Size, Mock_Packet_Type);
			arrange(testContext);

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: the handler was called and has the correct header
			ASSERT_TRUE(handlerContext.hasResponse());
			assertResponse(handlerContext);
		}

		void AssertReturnedValue(const std::vector<uint8_t>& expectedResult, const ionet::PacketPayload& payload) {
			ASSERT_EQ(1u, payload.buffers().size());
			ASSERT_EQ(expectedResult.size(), payload.buffers()[0].Size);

			EXPECT_EQ_MEMORY(expectedResult.data(), payload.buffers()[0].pData, expectedResult.size());
		}

		// endregion
	}

#define MAKE_BASIC_STATE_PATH_HANDLER_TEST(NAME) TEST(TEST_CLASS, NAME) { BasicHandlerTests::Assert##NAME(); }

	MAKE_BASIC_STATE_PATH_HANDLER_TEST(TooSmallPacketIsRejected)
	MAKE_BASIC_STATE_PATH_HANDLER_TEST(PacketWithWrongTypeIsRejected)
	MAKE_BASIC_STATE_PATH_HANDLER_TEST(PacketWithInvalidPayloadIsRejected)
	MAKE_BASIC_STATE_PATH_HANDLER_TEST(PacketWithTooSmallPayloadIsRejected)
	MAKE_BASIC_STATE_PATH_HANDLER_TEST(PacketWithNoPayloadIsRejected)

	TEST(TEST_CLASS, ValidPacketIsAcceptedWhenCacheIsEmpty) {
		// Assert: no element in cache, so response packet only contains header
		AssertPacketIsAccepted(
				[](const auto&) {},
				[](const auto& handlerContext) {
					test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader), Mock_Packet_Type);
				});
	}

	TEST(TEST_CLASS, NegativeProofIsReturnedWhenCacheDoesNotContainKey) {
		// Arrange:
		std::vector<uint8_t> expectedResponse;

		AssertPacketIsAccepted(
				[&expectedResponse](auto& testContext) {
					// - make tryLookup return negative proof
					expectedResponse = testContext.getCache().setLookupResult(false, 5);
				},
				[&expectedResponse](const auto& handlerContext) {
					// Assert: response packet contains serialized path
					test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader) + expectedResponse.size(), Mock_Packet_Type);
					AssertReturnedValue(expectedResponse, handlerContext.response());
				});
	}

	TEST(TEST_CLASS, PositiveProofIsReturnedWhenCacheContainsKey) {
		// Arrange:
		std::vector<uint8_t> expectedResponse;

		AssertPacketIsAccepted(
				[&expectedResponse](auto& testContext) {
					// - make tryLookup return positive proof
					expectedResponse = testContext.getCache().setLookupResult(true, 10);
				},
				[&expectedResponse](const auto& handlerContext) {
					// Assert: response packet contains serialized path
					test::AssertPacketHeader(handlerContext, sizeof(ionet::PacketHeader) + expectedResponse.size(), Mock_Packet_Type);
					AssertReturnedValue(expectedResponse, handlerContext.response());
				});
	}
}}
