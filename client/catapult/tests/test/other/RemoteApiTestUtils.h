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
#include "catapult/api/ApiTypes.h"
#include "catapult/utils/Casting.h"
#include "tests/test/core/mocks/MockPacketIo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region RemoteApiTests

	/// Group of tests for testing remote api classes.
	template<typename TApiTraits>
	class RemoteApiTests {
	public:
		static void AssertCanCreateApiWithRemotePublicKey() {
			// Arrange:
			auto pPacketIo = std::make_shared<mocks::MockPacketIo>();
			auto remotePublicKey = test::GenerateRandomByteArray<Key>();

			auto pApi = TApiTraits::Create(*pPacketIo, { remotePublicKey, "11.22.33.44" });

			// Act + Assert:
			EXPECT_EQ(remotePublicKey, pApi->remoteIdentity().PublicKey);
			EXPECT_EQ("11.22.33.44", pApi->remoteIdentity().Host);
		}
	};

	// endregion

	// region RemoteApiMethodTests

	/// Group of tests for testing remote api methods.
	template<typename TApiTraits>
	class RemoteApiMethodTests {
	private:
		using MockPacketIo = mocks::MockPacketIo;

	public:
		template<typename TTraits>
		static void ExceptionIsThrownWhenRemoteNodeWriteFails() {
			// Arrange: queue a write error
			auto pPacketIo = std::make_shared<MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Write_Error);
			auto pApi = TApiTraits::Create(*pPacketIo);

			// Act:
			AssertThrowsApiError([&api = *pApi] { TTraits::Invoke(api).get(); }, "write to remote node failed");

			// Assert:
			EXPECT_EQ(1u, pPacketIo->numWrites());
			EXPECT_EQ(0u, pPacketIo->numReads());
		}

		template<typename TTraits>
		static void ExceptionIsThrownWhenRemoteNodeReadFails() {
			// Arrange: queue a read error
			auto pPacketIo = std::make_shared<MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Read_Error);
			auto pApi = TApiTraits::Create(*pPacketIo);

			// Act:
			AssertThrowsApiError([&api = *pApi] { TTraits::Invoke(api).get(); }, "read from remote node failed");

			// Assert:
			EXPECT_EQ(1u, pPacketIo->numWrites());
			EXPECT_EQ(1u, pPacketIo->numReads());
		}

	private:
		template<typename TTraits>
		static void AssertMalformedPacket(const std::shared_ptr<ionet::Packet>& pResponsePacket) {
			// Arrange:
			auto pPacketIo = std::make_shared<MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Success, Wrap(pResponsePacket));
			auto pApi = TApiTraits::Create(*pPacketIo);

			// Act:
			AssertThrowsApiError(
					[&api = *pApi] { TTraits::Invoke(api).get(); },
					"remote node returned malformed packet");

			// Assert:
			EXPECT_EQ(1u, pPacketIo->numWrites());
			EXPECT_EQ(1u, pPacketIo->numReads());
		}

	public:
		template<typename TTraits>
		static void ExceptionIsThrownWhenRemoteNodeReturnsPacketWithWrongType() {
			// Act: return the wrong type of packet
			auto pResponsePacket = TTraits::CreateValidResponsePacket();
			pResponsePacket->Type = static_cast<ionet::PacketType>(utils::to_underlying_type(pResponsePacket->Type) + 1);
			AssertMalformedPacket<TTraits>(pResponsePacket);
		}

		template<typename TTraits>
		static void ExceptionIsThrownWhenRemoteNodeReturnsEmptyPacket() {
			// Arrange: return an empty packet
			auto pResponsePacket = TTraits::CreateValidResponsePacket();
			pResponsePacket->Size = sizeof(ionet::Packet);
			AssertMalformedPacket<TTraits>(pResponsePacket);
		}

		template<typename TTraits>
		static void ExceptionIsThrownWhenRemoteNodeReturnsMalformedPacket() {
			// Arrange: return a malformed packet
			AssertMalformedPacket<TTraits>(TTraits::CreateMalformedResponsePacket());
		}

		template<typename TTraits>
		static void WellFormedRequestIsWrittenToRemoteNode() {
			// Arrange:
			auto pPacketIo = std::make_shared<MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Success, Wrap(TTraits::CreateValidResponsePacket()));
			auto pApi = TApiTraits::Create(*pPacketIo);

			// Act:
			TTraits::Invoke(*pApi).get();

			// Assert:
			const auto& request = pPacketIo->writtenPacketAt<ionet::Packet>(0);
			TTraits::ValidateRequest(request);
			EXPECT_EQ(1u, pPacketIo->numWrites());
			EXPECT_EQ(1u, pPacketIo->numReads());
		}

		template<typename TTraits>
		static void WellFormedResponseFromRemoteNodeIsCoercedIntoDesiredType() {
			// Arrange:
			auto pResponsePacket = TTraits::CreateValidResponsePacket();

			auto pPacketIo = std::make_shared<MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Success, Wrap(pResponsePacket));
			auto pApi = TApiTraits::Create(*pPacketIo);

			// Act:
			auto result = TTraits::Invoke(*pApi).get();

			// Assert:
			TTraits::ValidateResponse(*pResponsePacket, result);
			EXPECT_EQ(1u, pPacketIo->numWrites());
			EXPECT_EQ(1u, pPacketIo->numReads());
		}

		template<typename TTraits>
		static void EmptyResponseIsConsideredValid() {
			// Arrange:
			auto pResponsePacket = TTraits::CreateValidResponsePacket();
			pResponsePacket->Size = sizeof(ionet::Packet);

			auto pPacketIo = std::make_shared<MockPacketIo>();
			pPacketIo->queueWrite(ionet::SocketOperationCode::Success);
			pPacketIo->queueRead(ionet::SocketOperationCode::Success, Wrap(pResponsePacket));
			auto pApi = TApiTraits::Create(*pPacketIo);

			// Act:
			auto result = TTraits::Invoke(*pApi).get();

			// Assert:
			EXPECT_EQ(0u, result.size());
			EXPECT_EQ(1u, pPacketIo->numWrites());
			EXPECT_EQ(1u, pPacketIo->numReads());
		}

	private:
		static void AssertThrowsApiError(const action& action, const char* expectedMessage) {
			try {
				action();
				FAIL() << "Expected catapult_api_error but no exception was thrown";
			} catch (const api::catapult_api_error& ex) {
				EXPECT_STREQ(expectedMessage, ex.what());
			} catch (...) {
				FAIL() << "Expected catapult_api_error but different exception was thrown";
			}
		}

		static MockPacketIo::GenerateReadPacket Wrap(const std::shared_ptr<ionet::Packet>& pPacket) {
			return [pPacket](const auto*) { return pPacket; };
		}
	};

	// endregion

#define MAKE_REMOTE_API_TEST(API_CLASS, TEST_NAME) \
	TEST(API_CLASS##Tests, TEST_NAME) { \
		test::RemoteApiTests<API_CLASS##Traits>::Assert##TEST_NAME(); \
	}

#define MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, TEST_NAME) \
	TEST(API_CLASS##Tests, TEST_NAME##_##API_FUNCTION) { \
		test::RemoteApiMethodTests<API_CLASS##Traits>::TEST_NAME<API_FUNCTION##Traits>(); \
	}

/// Adds all remote api tests for the specified api class (\a API_CLASS).
#define DEFINE_REMOTE_API_TESTS(API_CLASS) \
	MAKE_REMOTE_API_TEST(API_CLASS, CanCreateApiWithRemotePublicKey)

/// Adds all remote api tests for the specified api class (\a API_CLASS) and function (\a API_FUNCTION).
#define DEFINE_REMOTE_API_TESTS_BASIC(API_CLASS, API_FUNCTION) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, ExceptionIsThrownWhenRemoteNodeWriteFails) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, ExceptionIsThrownWhenRemoteNodeReadFails) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, ExceptionIsThrownWhenRemoteNodeReturnsPacketWithWrongType) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, ExceptionIsThrownWhenRemoteNodeReturnsMalformedPacket) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, WellFormedRequestIsWrittenToRemoteNode) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, WellFormedResponseFromRemoteNodeIsCoercedIntoDesiredType)

// for those requests where empty responses are not allowed
#define DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(API_CLASS, API_FUNCTION) \
	DEFINE_REMOTE_API_TESTS_BASIC(API_CLASS, API_FUNCTION) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, ExceptionIsThrownWhenRemoteNodeReturnsEmptyPacket)

// for those requests where empty responses are allowed
#define DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_VALID(API_CLASS, API_FUNCTION) \
	DEFINE_REMOTE_API_TESTS_BASIC(API_CLASS, API_FUNCTION) \
	MAKE_REMOTE_API_METHOD_TEST(API_CLASS, API_FUNCTION, EmptyResponseIsConsideredValid)
}}
