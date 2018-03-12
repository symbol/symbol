#include "src/builders/SecretProofBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS SecretProofBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::SecretProofTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedSecretProofTransaction>;

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const consumer<SecretProofBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			SecretProofBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6201, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Secret_Proof, pTransaction->Type);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(model::LockHashAlgorithm hashAlgorithm, const Hash512& secret, const RawBuffer& proof) {
			return [hashAlgorithm, &secret, &proof](const auto& transaction) {
				EXPECT_EQ(hashAlgorithm, transaction.HashAlgorithm);
				EXPECT_EQ(secret, transaction.Secret);
				EXPECT_TRUE(0 == std::memcmp(proof.pData, transaction.ProofPtr(), proof.Size));
			};
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				0u,
				[](const auto&) {},
				CreatePropertyChecker(model::LockHashAlgorithm::Op_Sha3, Hash512(), RawBuffer()));
	}

	// endregion

	// region additional transaction fields

	TRAITS_BASED_TEST(CanSetHashAlgorithm) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				0u,
				[](auto& builder) {
					builder.setHashAlgorithm(model::LockHashAlgorithm::Op_Hash_160);
				},
				CreatePropertyChecker(model::LockHashAlgorithm::Op_Hash_160, Hash512(), RawBuffer()));
	}

	TRAITS_BASED_TEST(CanSetSecret) {
		// Arrange:
		auto secret = test::GenerateRandomData<Hash512_Size>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				0u,
				[&secret](auto& builder) {
					builder.setSecret(secret);
				},
				CreatePropertyChecker(model::LockHashAlgorithm::Op_Sha3, secret, RawBuffer()));
	}

	TRAITS_BASED_TEST(CanSetProof) {
		// Arrange:
		auto proof = test::GenerateRandomData<20>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				proof.size(),
				[&proof](auto& builder) {
					builder.setProof(proof);
				},
				CreatePropertyChecker(model::LockHashAlgorithm::Op_Sha3, Hash512(), proof));
	}

	// endregion
}}
