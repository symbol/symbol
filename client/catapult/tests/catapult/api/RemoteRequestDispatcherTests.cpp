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

#include "catapult/api/RemoteRequestDispatcher.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

	namespace {
		// region custom packets

		static constexpr auto Generic_Request_Packet_Type = static_cast<ionet::PacketType>(11);
		static constexpr auto Generic_Response_Packet_Type = static_cast<ionet::PacketType>(7);

#pragma pack(push, 1)

		struct GenericRequestWithParams : public ionet::Packet {
			static constexpr auto Packet_Type = Generic_Request_Packet_Type;

			uint8_t Alpha;
			uint8_t Reserved1[7];
			uint64_t Beta;
			uint16_t Gamma;
		};

		struct GenericResponse : public ionet::Packet {
			static constexpr auto Packet_Type = Generic_Response_Packet_Type;

			uint8_t Foo;
		};

#pragma pack(pop)

		// endregion

		// region api traits

		// there are two sets of tests: one set for a request without parameters and one set for a request with parameters
		// the traits (and test traits) only differ in request handling

		struct BaseGenericApiTraits {
		public:
			using ResultType = uint8_t;
			static constexpr auto Packet_Type = GenericResponse::Packet_Type;

		public:
			explicit BaseGenericApiTraits(uint8_t multiplier = 1) : m_multiplier(multiplier)
			{}

			bool tryParseResult(const ionet::Packet& packet, ResultType& result) const {
				// intentionally do not use ionet::CoercePacket in order to test the type check in the dispatcher
				if (sizeof(GenericResponse) != packet.Size)
					return false;

				auto pResponse = static_cast<const GenericResponse*>(&packet);
				result = static_cast<ResultType>(pResponse->Foo * m_multiplier);
				return 0 == result % 2; // indicate only even results are valid
			}

		private:
			uint8_t m_multiplier;
		};

		struct GenericWithoutParametersApiTraits : public BaseGenericApiTraits {
			static constexpr auto Friendly_Name = "generic without parameters";

			using BaseGenericApiTraits::BaseGenericApiTraits;

			static auto CreateRequestPacketPayload() {
				return ionet::PacketPayload(GenericRequestWithParams::Packet_Type);
			}
		};

		struct GenericWithParametersApiTraits : public BaseGenericApiTraits {
			static constexpr auto Friendly_Name = "generic with parameters";

			static auto CreateRequestPacketPayload(uint8_t alpha, uint64_t beta, uint16_t gamma) {
				auto pPacket = ionet::CreateSharedPacket<GenericRequestWithParams>();
				pPacket->Alpha = alpha;
				pPacket->Beta = beta;
				pPacket->Gamma = gamma;
				return ionet::PacketPayload(pPacket);
			}
		};

		// endregion

		// region test traits

		struct BaseGenericTestTraits {
			static auto CreateValidResponsePacket() {
				auto pResponsePacket = ionet::CreateSharedPacket<GenericResponse>();
				pResponsePacket->Foo = 2; // even is valid
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				auto pResponsePacket = ionet::CreateSharedPacket<GenericResponse>();
				pResponsePacket->Foo = 1; // odd is invalid
				return pResponsePacket;
			}
		};

		struct GenericWithoutParametersTestTraits : public BaseGenericTestTraits {
			static auto Invoke(RemoteRequestDispatcher& api) {
				return api.dispatch(GenericWithoutParametersApiTraits());
			}

			static void ValidateResponse(const ionet::Packet&, const uint8_t& result) {
				EXPECT_EQ(2u, result);
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, Generic_Request_Packet_Type));
			}
		};

		struct GenericWithStatefulParsingTestTraits : public BaseGenericTestTraits {
			static auto Invoke(RemoteRequestDispatcher& api) {
				// Arrange: set a custom multiplier in the traits
				return api.dispatch(GenericWithoutParametersApiTraits(7));
			}

			static void ValidateResponse(const ionet::Packet&, const uint8_t& result) {
				// Assert: the result was multiplied by 7, which was set in the traits constructor
				EXPECT_EQ(14u, result);
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, Generic_Request_Packet_Type));
			}
		};

		struct GenericWithParametersTestTraits : public BaseGenericTestTraits {
			static auto Invoke(RemoteRequestDispatcher& api) {
				return api.dispatch(
						GenericWithParametersApiTraits(),
						static_cast<uint8_t>(6),
						static_cast<uint64_t>(2),
						static_cast<uint16_t>(5));
			}

			static void ValidateResponse(const ionet::Packet&, const uint8_t& result) {
				EXPECT_EQ(2u, result);
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				const auto* pRequest = ionet::CoercePacket<GenericRequestWithParams>(&packet);
				ASSERT_TRUE(!!pRequest);
				EXPECT_EQ(6u, pRequest->Alpha);
				EXPECT_EQ(2u, pRequest->Beta);
				EXPECT_EQ(5u, pRequest->Gamma);
			}
		};

		// endregion

		struct RemoteRequestDispatcherTraits {
			static auto Create(ionet::PacketIo& packetIo) {
				return std::make_unique<RemoteRequestDispatcher>(packetIo);
			}
		};
	}

	// - zero parameter requests
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteRequestDispatcher, GenericWithoutParametersTest)

	// - state-dependent response parsing
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteRequestDispatcher, GenericWithStatefulParsingTest)

	// - multi parameter requests
	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteRequestDispatcher, GenericWithParametersTest)
}}
