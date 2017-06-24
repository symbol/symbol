#include "catapult/net/Challenge.h"
#include "catapult/crypto/Signer.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace net {

	namespace {
		void AssertRandomChallengeGenerator(const std::function<Challenge ()>& generate) {
			// Arrange:
			static const auto Num_Challenges = 100u;
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

	TEST(ChallengeTests, GenerateServerChallengeRequestCreatesAppropriateRequest) {
		// Act:
		auto pRequest = GenerateServerChallengeRequest();

		// Assert:
		EXPECT_EQ(sizeof(ServerChallengeRequest), pRequest->Size);
		EXPECT_EQ(ionet::PacketType::Server_Challenge, pRequest->Type);
		EXPECT_NE(Challenge{}, pRequest->Challenge); // challenge is non-zero
	}

	TEST(ChallengeTests, GenerateServerChallengeRequestCreatesRandomChallenge) {
		// Assert:
		AssertRandomChallengeGenerator([]() { return GenerateServerChallengeRequest()->Challenge; });
	}

	// endregion

	//region GenerateServerChallengeResponse

	TEST(ChallengeTests, GenerateServerChallengeResponseCreatesAppropriateResponse) {
		// Arrange:
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();

		// Act:
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair);

		// Assert:
		EXPECT_EQ(sizeof(ServerChallengeResponse), pResponse->Size);
		EXPECT_EQ(ionet::PacketType::Server_Challenge, pResponse->Type);
		EXPECT_NE(Challenge{}, pResponse->Challenge); // challenge is non-zero
		EXPECT_NE(pRequest->Challenge, pResponse->Challenge); // challenge is not the same as the request challenge
		EXPECT_TRUE(crypto::Verify(pResponse->PublicKey, pRequest->Challenge, pResponse->Signature));
		EXPECT_EQ(keyPair.publicKey(), pResponse->PublicKey);
	}

	TEST(ChallengeTests, GenerateServerChallengeResponseCreatesRandomChallenge) {
		// Assert:
		AssertRandomChallengeGenerator([]() {
			return GenerateServerChallengeResponse(ServerChallengeRequest(), test::GenerateKeyPair())->Challenge;
		});
	}

	// endregion

	// region VerifyServerChallengeResponse

	TEST(ChallengeTests, VerifyServerChallengeResponseReturnsTrueForGoodResponse) {
		// Arrange:
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair);

		// Act:
		auto isVerified = VerifyServerChallengeResponse(*pResponse, pRequest->Challenge);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(ChallengeTests, VerifyServerChallengeResponseReturnsFalseForBadResponse) {
		// Arrange: invalidate the signature
		auto pRequest = GenerateServerChallengeRequest();
		auto keyPair = test::GenerateKeyPair();
		auto pResponse = GenerateServerChallengeResponse(*pRequest, keyPair);
		pResponse->Signature[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyServerChallengeResponse(*pResponse, pRequest->Challenge);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	// endregion

	// region GenerateClientChallengeResponse

	TEST(ChallengeTests, GenerateClientChallengeResponseCreatesAppropriateResponse) {
		// Arrange:
		auto pRequest = GenerateServerChallengeResponse(*GenerateServerChallengeRequest(), test::GenerateKeyPair());
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

	TEST(ChallengeTests, VerifyClientChallengeResponseReturnsTrueForGoodResponse) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		auto pRequest = GenerateServerChallengeResponse(*GenerateServerChallengeRequest(), keyPair);
		auto pResponse = GenerateClientChallengeResponse(*pRequest, keyPair);

		// Act:
		auto isVerified = VerifyClientChallengeResponse(*pResponse, keyPair.publicKey(), pRequest->Challenge);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(ChallengeTests, VerifyClientChallengeResponseReturnsFalseForBadResponse) {
		// Arrange: invalidate the signature
		auto keyPair = test::GenerateKeyPair();
		auto pRequest = GenerateServerChallengeResponse(*GenerateServerChallengeRequest(), keyPair);
		auto pResponse = GenerateClientChallengeResponse(*pRequest, keyPair);
		pResponse->Signature[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyClientChallengeResponse(*pResponse, keyPair.publicKey(), pRequest->Challenge);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	// endregion
}}
