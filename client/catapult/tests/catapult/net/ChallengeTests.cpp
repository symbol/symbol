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

#include "catapult/net/Challenge.h"
#include "catapult/crypto/Signer.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

#define TEST_CLASS ChallengeTests

	namespace {
		void AssertRandomChallengeGenerator(const supplier<Challenge>& generate) {
			// Arrange:
			constexpr auto Num_Challenges = 100u;
			std::set<Challenge> challenges;

			// Act: generate Num_Challenges
			for (auto i = 0u; i < Num_Challenges; ++i) {
				// - only add the challenge if it has not already been added
				auto challenge = generate();
				if (challenges.cend() != challenges.find(challenge))
					continue;

				challenges.insert(challenge);
			}

			// Assert: all challenges are unique
			EXPECT_EQ(Num_Challenges, challenges.size());
		}
	}

	// region GenerateServerChallengeRequest

	TEST(TEST_CLASS, GenerateServerChallengeRequestCreatesAppropriateRequest) {
		// Act:
		auto pRequest = GenerateServerChallengeRequest();

		// Assert:
		EXPECT_EQ(sizeof(ServerChallengeRequest), pRequest->Size);
		EXPECT_EQ(ionet::PacketType::Server_Challenge, pRequest->Type);
		EXPECT_NE(Challenge(), pRequest->Challenge); // challenge is non-zero
	}

	TEST(TEST_CLASS, GenerateServerChallengeRequestCreatesRandomChallenge) {
		AssertRandomChallengeGenerator([]() { return GenerateServerChallengeRequest()->Challenge; });
	}

	// endregion

	// region GenerateServerChallengeResponse

	TEST(TEST_CLASS, GenerateServerChallengeResponseCreatesAppropriateResponse) {
		// Arrange:
		constexpr auto Challenge_Size = std::tuple_size_v<Challenge>;
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();

		// Act:
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair, ionet::ConnectionSecurityMode::Signed);

		// - construct expected signed data
		auto signedData = std::vector<uint8_t>(Challenge_Size + 1);
		std::memcpy(signedData.data(), pRequest->Challenge.data(), Challenge_Size);
		signedData[Challenge_Size] = utils::to_underlying_type(ionet::ConnectionSecurityMode::Signed);

		// Assert:
		EXPECT_EQ(sizeof(ServerChallengeResponse), pResponse->Size);
		EXPECT_EQ(ionet::PacketType::Server_Challenge, pResponse->Type);
		EXPECT_NE(Challenge(), pResponse->Challenge); // challenge is non-zero
		EXPECT_NE(pRequest->Challenge, pResponse->Challenge); // challenge is not the same as the request challenge
		EXPECT_TRUE(crypto::Verify(pResponse->PublicKey, signedData, pResponse->Signature));
		EXPECT_EQ(keyPair.publicKey(), pResponse->PublicKey);
	}

	TEST(TEST_CLASS, GenerateServerChallengeResponseCreatesRandomChallenge) {
		AssertRandomChallengeGenerator([]() {
			auto securityMode = ionet::ConnectionSecurityMode::None;
			auto pResponse = GenerateServerChallengeResponse(ServerChallengeRequest(), test::GenerateKeyPair(), securityMode);
			return pResponse->Challenge;
		});
	}

	// endregion

	// region VerifyServerChallengeResponse

	TEST(TEST_CLASS, VerifyServerChallengeResponseReturnsTrueForGoodResponse) {
		// Arrange:
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair, ionet::ConnectionSecurityMode::None);

		// Act:
		auto isVerified = VerifyServerChallengeResponse(*pResponse, pRequest->Challenge);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, VerifyServerChallengeResponseReturnsFalseForBadResponseWithCorruptSignature) {
		// Arrange: invalidate the signature
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair, ionet::ConnectionSecurityMode::None);
		pResponse->Signature[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyServerChallengeResponse(*pResponse, pRequest->Challenge);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(TEST_CLASS, VerifyServerChallengeResponseReturnsFalseForBadResponseWithCorruptSecurityMode) {
		// Arrange: change the security mode
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair, ionet::ConnectionSecurityMode::None);
		pResponse->SecurityMode = ionet::ConnectionSecurityMode::Signed;

		// Act:
		auto isVerified = VerifyServerChallengeResponse(*pResponse, pRequest->Challenge);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	// endregion

	// region GenerateClientChallengeResponse

	TEST(TEST_CLASS, GenerateClientChallengeResponseCreatesAppropriateResponse) {
		// Arrange:
		auto pServerRequest = GenerateServerChallengeRequest();
		auto pRequest = GenerateServerChallengeResponse(*pServerRequest, test::GenerateKeyPair(), ionet::ConnectionSecurityMode::None);
		auto keyPair = test::GenerateKeyPair();

		// Act:
		auto pResponse = GenerateClientChallengeResponse(*pRequest, keyPair);

		// Assert:
		EXPECT_EQ(sizeof(ClientChallengeResponse), pResponse->Size);
		EXPECT_EQ(ionet::PacketType::Client_Challenge, pResponse->Type);
		EXPECT_TRUE(crypto::Verify(keyPair.publicKey(), pRequest->Challenge, pResponse->Signature));
	}

	// endregion

	// region VerifyClientChallengeResponse

	TEST(TEST_CLASS, VerifyClientChallengeResponseReturnsTrueForGoodResponse) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		auto pServerRequest = GenerateServerChallengeRequest();
		auto pRequest = GenerateServerChallengeResponse(*pServerRequest, keyPair, ionet::ConnectionSecurityMode::None);
		auto pResponse = GenerateClientChallengeResponse(*pRequest, keyPair);

		// Act:
		auto isVerified = VerifyClientChallengeResponse(*pResponse, keyPair.publicKey(), pRequest->Challenge);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, VerifyClientChallengeResponseReturnsFalseForBadResponse) {
		// Arrange: invalidate the signature
		auto keyPair = test::GenerateKeyPair();
		auto pServerRequest = GenerateServerChallengeRequest();
		auto pRequest = GenerateServerChallengeResponse(*pServerRequest, keyPair, ionet::ConnectionSecurityMode::None);
		auto pResponse = GenerateClientChallengeResponse(*pRequest, keyPair);
		pResponse->Signature[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyClientChallengeResponse(*pResponse, keyPair.publicKey(), pRequest->Challenge);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	// endregion
}}
