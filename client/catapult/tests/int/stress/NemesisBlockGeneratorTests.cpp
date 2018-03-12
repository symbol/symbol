#include "sdk/src/extensions/BlockExtensions.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/int/stress/test/EntityDump.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/EntityFactory.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"

using catapult::crypto::KeyPair;

namespace catapult {

#define TEST_CLASS NemesisBlockGeneratorTests

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
		constexpr Amount Nemesis_Amount(9000000000ull / CountOf(test::Mijin_Test_Private_Keys) * 1000000ull);
	}

	TEST(TEST_CLASS, CreateTransaction) {
		// Arrange:
		auto signer = KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);
		auto recipient = KeyPair::FromString(test::Mijin_Test_Private_Keys[0]);

		// Act:
		auto pTransaction = test::CreateTransferTransaction(signer, recipient.publicKey(), Amount(1234));
		test::EntityDump(*pTransaction);

		// Assert:
		EXPECT_TRUE(extensions::VerifyTransactionSignature(*pTransaction));
	}

	TEST(TEST_CLASS, CreateNemesisBlockTransactions) {
		// Arrange:
		auto signer = KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);

		for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys) {
			auto recipient = KeyPair::FromString(pRecipientPrivateKeyString);
			auto pTransaction = test::CreateTransferTransaction(signer, recipient.publicKey(), Nemesis_Amount);
#ifdef _DEBUG
			test::EntityDump(*pTransaction);
#endif
			EXPECT_TRUE(extensions::VerifyTransactionSignature(*pTransaction));
		}
	}

	namespace {
		auto CreateNemesisBlock() {
			auto signer = KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);

			model::Transactions transactions;
			for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys) {
				auto recipient = KeyPair::FromString(pRecipientPrivateKeyString);
				auto pTransfer = test::CreateTransferTransaction(signer, recipient.publicKey(), Nemesis_Amount);
				pTransfer->Fee = Amount(0);
				extensions::SignTransaction(signer, *pTransfer);
				transactions.push_back(std::move(pTransfer));
			}

			model::PreviousBlockContext context;
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), transactions);
			test::SignBlock(signer, *pBlock);
			return pBlock;
		}

		void VerifyNemesisBlock(const model::Block& block) {
			EXPECT_EQ(extensions::VerifyFullBlockResult::Success, extensions::BlockExtensions().verifyFullBlock(block));
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
