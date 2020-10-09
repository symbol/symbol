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

#include "sdk/src/extensions/BlockExtensions.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/NetworkIdentifier.h"
#include "tests/int/stress/test/EntityDump.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TestNetworkConstants.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS NemesisBlockGeneratorIntegrityTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
		constexpr Amount Nemesis_Amount(9000000000ull / CountOf(test::Test_Network_Private_Keys) * 1000000ull);

		bool VerifyNemesisNetworkTransactionSignature(const model::Transaction& transaction) {
			return extensions::TransactionExtensions(test::GetNemesisGenerationHashSeed()).verify(transaction);
		}
	}

	TEST(TEST_CLASS, CreateTransaction) {
		// Arrange:
		auto signer = crypto::KeyPair::FromString(test::Test_Network_Nemesis_Private_Key);
		auto recipient = crypto::KeyPair::FromString(test::Test_Network_Private_Keys[0]);

		// Act:
		auto pTransaction = test::CreateTransferTransaction(signer, recipient.publicKey(), Amount(1234));
		test::EntityDump(*pTransaction);

		// Assert:
		EXPECT_TRUE(VerifyNemesisNetworkTransactionSignature(*pTransaction));
	}

	TEST(TEST_CLASS, CreateNemesisBlockTransactions) {
		// Arrange:
		auto signer = crypto::KeyPair::FromString(test::Test_Network_Nemesis_Private_Key);

		for (const auto* pRecipientPrivateKeyString : test::Test_Network_Private_Keys) {
			auto recipient = crypto::KeyPair::FromString(pRecipientPrivateKeyString);

			// Act:
			auto pTransaction = test::CreateTransferTransaction(signer, recipient.publicKey(), Nemesis_Amount);
#ifdef _DEBUG
			test::EntityDump(*pTransaction);
#endif

			// Assert:
			EXPECT_TRUE(VerifyNemesisNetworkTransactionSignature(*pTransaction));
		}
	}

	namespace {
		auto CreateNemesisBlock() {
			auto signer = crypto::KeyPair::FromString(test::Test_Network_Nemesis_Private_Key);
			auto generationHashSeed = test::GetNemesisGenerationHashSeed();

			model::Transactions transactions;
			for (const auto* pRecipientPrivateKeyString : test::Test_Network_Private_Keys) {
				auto recipient = crypto::KeyPair::FromString(pRecipientPrivateKeyString);
				auto pTransfer = test::CreateTransferTransaction(signer, recipient.publicKey(), Nemesis_Amount);
				pTransfer->MaxFee = Amount(0);
				extensions::TransactionExtensions(generationHashSeed).sign(signer, *pTransfer);
				transactions.push_back(std::move(pTransfer));
			}

			model::PreviousBlockContext context;
			context.GenerationHash = generationHashSeed.copyTo<GenerationHash>();
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), transactions);
			extensions::BlockExtensions(generationHashSeed).signFullBlock(signer, *pBlock);
			return pBlock;
		}

		void VerifyNemesisBlock(const model::Block& block) {
			auto blockExtensions = extensions::BlockExtensions(test::GetNemesisGenerationHashSeed());
			auto verifyResult = blockExtensions.verifyFullBlock(block);
			EXPECT_EQ(extensions::VerifyFullBlockResult::Success, verifyResult);
		}
	}

	TEST(TEST_CLASS, CreateNemesisBlock) {
		// Act:
		auto pBlock = CreateNemesisBlock();
		test::EntityDump(*pBlock);

		// Assert:
		VerifyNemesisBlock(*pBlock);
	}
}
