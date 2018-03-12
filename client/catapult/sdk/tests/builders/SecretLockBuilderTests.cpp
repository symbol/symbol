#include "src/builders/SecretLockBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS SecretLockBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::SecretLockTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedSecretLockTransaction>;

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				const consumer<SecretLockBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			SecretLockBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(0u, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6201, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Secret_Lock, pTransaction->Type);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(
				const model::Mosaic& mosaic,
				BlockDuration duration,
				model::LockHashAlgorithm hashAlgorithm,
				const Hash512& secret,
				const Address& recipient) {
			return [&mosaic, duration, hashAlgorithm, &secret, &recipient](const auto& transaction) {
				EXPECT_EQ(mosaic.MosaicId, transaction.Mosaic.MosaicId);
				EXPECT_EQ(mosaic.Amount, transaction.Mosaic.Amount);
				EXPECT_EQ(duration, transaction.Duration);
				EXPECT_EQ(hashAlgorithm, transaction.HashAlgorithm);
				EXPECT_EQ(secret, transaction.Secret);
				EXPECT_EQ(recipient, transaction.Recipient);
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
				[](const auto&) {},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(), model::LockHashAlgorithm::Op_Sha3, Hash512(), Address()));
	}

	// endregion

	// region additional transaction fields

	TRAITS_BASED_TEST(CanSetMosaic) {
		// Arrange:
		model::Mosaic mosaic{ MosaicId(123), Amount(234) };

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[&mosaic](auto& builder) {
					builder.setMosaic(mosaic.MosaicId, mosaic.Amount);
				},
				CreatePropertyChecker(mosaic, BlockDuration(), model::LockHashAlgorithm::Op_Sha3, Hash512(), Address()));
	}

	TRAITS_BASED_TEST(CanSetBlockDuration) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[](auto& builder) {
					builder.setDuration(BlockDuration(123));
				},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(123), model::LockHashAlgorithm::Op_Sha3, Hash512(), Address()));
	}

	TRAITS_BASED_TEST(CanSetHashAlgorithm) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[](auto& builder) {
					builder.setHashAlgorithm(model::LockHashAlgorithm::Op_Hash_160);
				},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(), model::LockHashAlgorithm::Op_Hash_160, Hash512(), Address()));
	}

	TRAITS_BASED_TEST(CanSetSecret) {
		// Arrange:
		auto secret = test::GenerateRandomData<Hash512_Size>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[&secret](auto& builder) {
					builder.setSecret(secret);
				},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(), model::LockHashAlgorithm::Op_Sha3, secret, Address()));
	}

	TRAITS_BASED_TEST(CanSetRecipient) {
		// Arrange:
		auto recipient = test::GenerateRandomData<Address_Decoded_Size>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[&recipient](auto& builder) {
					builder.setRecipient(recipient);
				},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(), model::LockHashAlgorithm::Op_Sha3, Hash512(), recipient));
	}

	// endregion
}}
