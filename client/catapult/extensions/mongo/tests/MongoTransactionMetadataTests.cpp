#include "mongo/src/MongoTransactionMetadata.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS MongoTransactionMetadataTests

	namespace {
		model::TransactionElement CreateTransactionElement(const model::Transaction& transaction) {
			model::TransactionElement transactionElement(transaction);
			transactionElement.EntityHash = test::GenerateRandomData<Hash256_Size>();
			transactionElement.MerkleComponentHash = test::GenerateRandomData<Hash256_Size>();
			transactionElement.OptionalExtractedAddresses = std::make_shared<model::AddressSet>(test::GenerateRandomAddressSet(3));
			return transactionElement;
		}
	}

	TEST(TEST_CLASS, CanCreateMetadataAroundTransactionElement) {
		// Arrange:
		auto pTransaction = test::GenerateRandomTransaction();
		auto element = CreateTransactionElement(*pTransaction);

		// Act:
		auto metadata = MongoTransactionMetadata(element);

		// Assert:
		EXPECT_EQ(element.EntityHash, metadata.EntityHash);
		EXPECT_EQ(element.MerkleComponentHash, metadata.MerkleComponentHash);
		EXPECT_EQ(element.OptionalExtractedAddresses.get(), &metadata.Addresses);
		EXPECT_EQ(Height(), metadata.Height);
		EXPECT_EQ(0u, metadata.Index);
	}

	TEST(TEST_CLASS, CanCreateMetadataAroundTransactionElementAndContainingBlockInformation) {
		// Arrange:
		auto pTransaction = test::GenerateRandomTransaction();
		auto element = CreateTransactionElement(*pTransaction);

		// Act:
		auto metadata = MongoTransactionMetadata(element, Height(17), 12);

		// Assert:
		EXPECT_EQ(element.EntityHash, metadata.EntityHash);
		EXPECT_EQ(element.MerkleComponentHash, metadata.MerkleComponentHash);
		EXPECT_EQ(element.OptionalExtractedAddresses.get(), &metadata.Addresses);
		EXPECT_EQ(Height(17), metadata.Height);
		EXPECT_EQ(12u, metadata.Index);
	}

	TEST(TEST_CLASS, CanCreateMetadataAroundTransactionInfo) {
		// Arrange:
		auto transactionInfo = test::CreateRandomTransactionInfo();

		// Act:
		auto metadata = MongoTransactionMetadata(transactionInfo);

		// Assert:
		EXPECT_EQ(transactionInfo.EntityHash, metadata.EntityHash);
		EXPECT_EQ(transactionInfo.MerkleComponentHash, metadata.MerkleComponentHash);
		EXPECT_EQ(transactionInfo.OptionalExtractedAddresses.get(), &metadata.Addresses);
		EXPECT_EQ(Height(), metadata.Height);
		EXPECT_EQ(0u, metadata.Index);
	}

	TEST(TEST_CLASS, ObjectIdIsUniqueAcrossInstances) {
		// Arrange:
		auto pTransaction = test::GenerateRandomTransaction();
		auto element = CreateTransactionElement(*pTransaction);

		// Act:
		auto metadata1 = MongoTransactionMetadata(element);
		auto metadata2 = MongoTransactionMetadata(element);

		// Assert:
		EXPECT_NE(metadata1.ObjectId, metadata2.ObjectId);
	}
}}
