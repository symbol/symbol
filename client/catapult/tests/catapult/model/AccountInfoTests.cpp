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

#include "catapult/model/AccountInfo.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AccountInfoTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(uint32_t) // size
				+ Address_Decoded_Size // account address
				+ sizeof(Height) // address height
				+ Key_Size // public key
				+ sizeof(Height) // public key height
				+ Importance_History_Size * sizeof(Importance) // account importances
				+ Importance_History_Size * sizeof(ImportanceHeight) // account importance heights
				+ sizeof(uint16_t); // number of mosaics

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(AccountInfo));
		EXPECT_EQ(127u, sizeof(AccountInfo));
	}

	TEST(TEST_CLASS, AccountInfoMaxSizeHasCorrectValue) {
		// Arrange:
		auto expectedMaxSize =
				sizeof(AccountInfo)
				+ sizeof(Mosaic) * 65535; // size of mosaic * maximum number of mosaics

		// Assert:
		EXPECT_EQ(expectedMaxSize, AccountInfo_Max_Size);
		EXPECT_EQ(127u + 16 * 65535, AccountInfo_Max_Size);
	}

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		AccountInfo accountInfo;
		accountInfo.Size = 0;
		accountInfo.MosaicsCount = 100;

		// Act:
		auto realSize = AccountInfo::CalculateRealSize(accountInfo);

		// Assert:
		EXPECT_EQ(sizeof(AccountInfo) + 100 * sizeof(Mosaic), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		AccountInfo accountInfo;
		accountInfo.Size = 0;
		test::SetMaxValue(accountInfo.MosaicsCount);

		// Act:
		auto realSize = AccountInfo::CalculateRealSize(accountInfo);

		// Assert:
		EXPECT_EQ(sizeof(AccountInfo) + accountInfo.MosaicsCount * sizeof(Mosaic), realSize);
		EXPECT_GE(std::numeric_limits<uint32_t>::max(), realSize);
	}

	// endregion

	// region data pointers

	namespace {
		struct AccountInfoTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = sizeof(AccountInfo) + count * sizeof(Mosaic);
				auto pAccountInfo = utils::MakeUniqueWithSize<AccountInfo>(entitySize);
				pAccountInfo->Size = entitySize;
				pAccountInfo->MosaicsCount = count;
				return pAccountInfo;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.MosaicsPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, AccountInfoTraits) // MosaicsPtr

	// endregion

	// region FromAddress

	TEST(TEST_CLASS, FromAddressReturnsAccountInfoWithInitializedFields) {
		// Arrange:
		auto address = test::GenerateRandomData<Address_Decoded_Size>();

		// Act:
		auto pAccountInfo = AccountInfo::FromAddress(address);

		// Assert:
		EXPECT_EQ(sizeof(model::AccountInfo), pAccountInfo->Size);
		EXPECT_EQ(address, pAccountInfo->Address);
		EXPECT_EQ(Height(0), pAccountInfo->AddressHeight);
		EXPECT_EQ(Key(), pAccountInfo->PublicKey);
		EXPECT_EQ(Height(0), pAccountInfo->PublicKeyHeight);
		EXPECT_EQ(0, pAccountInfo->MosaicsCount);

		for (auto i = 0u; i < Importance_History_Size; ++i) {
			const auto message = "importance at " + std::to_string(i);
			EXPECT_EQ(Importance(0), pAccountInfo->Importances[i]) << message;
			EXPECT_EQ(model::ImportanceHeight(0), pAccountInfo->ImportanceHeights[i]) << message;
		}
	}

	// endregion
}}
