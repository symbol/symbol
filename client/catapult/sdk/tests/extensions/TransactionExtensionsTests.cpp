#include "src/extensions/TransactionExtensions.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS TransactionExtensionsTests

namespace catapult { namespace extensions {

	namespace {
		struct NormalTraits {
			static constexpr size_t Entity_Size = sizeof(model::Transaction);
		};
		struct LargeTraits {
			static constexpr size_t Entity_Size = sizeof(model::Transaction) + 123;
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Normal) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Large) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LargeTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(CannotValidateUnsignedTransaction) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);

		// Act + Assert:
		EXPECT_FALSE(VerifyTransactionSignature(*pEntity));
	}

	TRAITS_BASED_TEST(SignedTransactionValidates) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		(*pEntity).Signer = signer.publicKey();
		SignTransaction(signer, *pEntity);

		// Act + Assert:
		EXPECT_TRUE(VerifyTransactionSignature(*pEntity));
	}

	TRAITS_BASED_TEST(CannotValidateAlteredSignedTransaction) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		(*pEntity).Signer = signer.publicKey();
		SignTransaction(signer, *pEntity);

		(*pEntity).Deadline = Timestamp(pEntity->Deadline.unwrap() ^ 0xFFFF'FFFF'FFFF'FFFFull);

		// Act + Assert:
		EXPECT_FALSE(VerifyTransactionSignature(*pEntity));
	}

	TRAITS_BASED_TEST(CannotValidateSignedTransactionWithAlteredSignature) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		(*pEntity).Signer = signer.publicKey();
		SignTransaction(signer, *pEntity);

		(*pEntity).Signature[0] ^= 0xFFu;

		// Act + Assert:
		EXPECT_FALSE(VerifyTransactionSignature(*pEntity));
	}
}}
