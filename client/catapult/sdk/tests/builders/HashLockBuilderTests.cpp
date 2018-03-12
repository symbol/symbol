#include "src/builders/HashLockBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS HashLockBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::HashLockTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedHashLockTransaction>;

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				const consumer<HashLockBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			HashLockBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(0u, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6201, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Hash_Lock, pTransaction->Type);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(const model::Mosaic& mosaic, BlockDuration duration, const Hash256& hash) {
			return [&mosaic, duration, &hash](const auto& transaction) {
				EXPECT_EQ(mosaic.MosaicId, transaction.Mosaic.MosaicId);
				EXPECT_EQ(mosaic.Amount, transaction.Mosaic.Amount);
				EXPECT_EQ(duration, transaction.Duration);
				EXPECT_EQ(hash, transaction.Hash);
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
				CreatePropertyChecker(model::Mosaic(), BlockDuration(0), Hash256()));
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
				CreatePropertyChecker(mosaic, BlockDuration(0), Hash256()));
	}

	TRAITS_BASED_TEST(CanSetBlockDuration) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[](auto& builder) {
					builder.setDuration(BlockDuration(123));
				},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(123), Hash256()));
	}

	TRAITS_BASED_TEST(CanSetHash) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				[&hash](auto& builder) {
					builder.setHash(hash);
				},
				CreatePropertyChecker(model::Mosaic(), BlockDuration(0), hash));
	}

	// endregion
}}
