#include "src/MongoTransactionMetadata.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MongoTransactionMetadataTests

namespace catapult { namespace mongo { namespace plugins {

	TEST(TEST_CLASS, CanCreateMetadataAroundHashes) {
		// Arrange:
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto metadata = MongoTransactionMetadata(entityHash, merkleComponentHash);

		// Assert:
		EXPECT_EQ(entityHash, metadata.EntityHash);
		EXPECT_EQ(merkleComponentHash, metadata.MerkleComponentHash);
		EXPECT_EQ(Height(), metadata.Height);
		EXPECT_EQ(0u, metadata.Index);
	}

	TEST(TEST_CLASS, CanCreateMetadataAroundHashesAndContainingBlockInformation) {
		// Arrange:
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto metadata = MongoTransactionMetadata(entityHash, merkleComponentHash, Height(17), 12);

		// Assert:
		EXPECT_EQ(entityHash, metadata.EntityHash);
		EXPECT_EQ(merkleComponentHash, metadata.MerkleComponentHash);
		EXPECT_EQ(Height(17), metadata.Height);
		EXPECT_EQ(12u, metadata.Index);
	}

	TEST(TEST_CLASS, ObjectIdIsUniqueAcrossInstances) {
		// Arrange:
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();

		// Act:
		auto metadata1 = MongoTransactionMetadata(entityHash, merkleComponentHash);
		auto metadata2 = MongoTransactionMetadata(entityHash, merkleComponentHash);

		// Assert:
		EXPECT_NE(metadata1.ObjectId, metadata2.ObjectId);
	}
}}}
