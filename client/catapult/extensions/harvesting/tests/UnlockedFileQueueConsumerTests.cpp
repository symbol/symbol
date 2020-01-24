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

#include "harvesting/src/UnlockedFileQueueConsumer.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/FileQueue.h"
#include "harvesting/tests/test/UnlockedTestEntry.h"
#include "tests/test/crypto/EncryptionTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedFileQueueConsumerTests

	// region decrypt unlocked entry

	namespace {
		void AssertFailureWhenDecryptedDataHasInvalidSize(size_t dataSize) {
			// Arrange:
			auto clearText = test::GenerateRandomVector(dataSize);
			auto recipientKeyPair = test::GenerateKeyPair();
			auto encryptedWithKey = test::GenerateEphemeralAndEncrypt(clearText, recipientKeyPair.publicKey());

			// Act:
			auto decryptedPair = TryDecryptUnlockedEntry(encryptedWithKey, recipientKeyPair);

			// Assert:
			EXPECT_FALSE(decryptedPair.second);
		}
	}

	TEST(TEST_CLASS, TryDecryptUnlockedEntry_FailsWhenDecryptedDataIsTooSmall) {
		AssertFailureWhenDecryptedDataHasInvalidSize(Key::Size - 1);
	}

	TEST(TEST_CLASS, TryDecryptUnlockedEntry_FailsWhenDecryptedDataIsTooLarge) {
		AssertFailureWhenDecryptedDataHasInvalidSize(Key::Size + 1);
	}

	TEST(TEST_CLASS, TryDecryptUnlockedEntry_SucceedsWhenDecryptedDataHasProperSize) {
		// Arrange:
		auto clearText = test::GenerateRandomByteArray<Key>();
		auto recipientKeyPair = test::GenerateKeyPair();
		auto encryptedWithKey = test::GenerateEphemeralAndEncrypt(clearText, recipientKeyPair.publicKey());

		// Act:
		auto decryptedPair = TryDecryptUnlockedEntry(encryptedWithKey, recipientKeyPair);

		// Assert:
		EXPECT_TRUE(decryptedPair.second);
		EXPECT_EQ_MEMORY(clearText.data(), decryptedPair.first.data(), Key::Size);
	}

	// endregion

	namespace {
		// region utils

		auto GenerateKeys(size_t numKeys) {
			std::vector<Key> randomPrivateKeys;
			for (auto i = 0u; i < numKeys; ++i)
				randomPrivateKeys.emplace_back(test::GenerateRandomByteArray<Key>());

			return randomPrivateKeys;
		}

		auto SerializeUnlockedEntryMessage(const UnlockedEntryMessage& message) {
			std::vector<uint8_t> messageBuffer(1 + message.EncryptedEntry.Size);
			messageBuffer[0] = static_cast<uint8_t>(message.Direction);
			std::memcpy(messageBuffer.data() + 1, message.EncryptedEntry.pData, message.EncryptedEntry.Size);
			return messageBuffer;
		}

		// endregion

		// region test context

		class TestContext {
		public:
			TestContext() : m_keyPair(test::GenerateKeyPair())
			{}

		public:
			void write(const std::vector<std::vector<uint8_t>>& messages) {
				io::FileQueueWriter writer(m_directoryGuard.name());
				for (const auto& message : messages) {
					writer.write(message);
					writer.flush();
				}
			}

			void runConsumerAndAssert(
					const std::vector<Key>& keys,
					const std::vector<std::vector<uint8_t>>& messages,
					std::initializer_list<size_t> validIndexes) {
				// Act:
				config::CatapultDirectory directory(m_directoryGuard.name());
				std::vector<std::vector<uint8_t>> collectedMessages;
				std::vector<crypto::KeyPair> collectedKeyPairs;
				UnlockedFileQueueConsumer(directory, m_keyPair, [&collectedMessages, &collectedKeyPairs](
						const auto& message,
						auto&& harvesterKeyPair) {
					collectedMessages.emplace_back(SerializeUnlockedEntryMessage(message));
					collectedKeyPairs.emplace_back(std::move(harvesterKeyPair));
				});

				// Assert:
				assertForwardedMessages(collectedMessages, messages, validIndexes);
				assertKeyPairs(collectedKeyPairs, keys, validIndexes);
			}

		public:
			enum class InvalidMessageFlag { Invalid_Padding, Invalid_Decrypted_Data_Size };
			enum class Sizes { Underflow, Normal, Overflow };

			auto prepareMessage(const Key& randomPrivate, test::EncryptionMutationFlag encryptionMutationFlag) {
				auto entry = test::PrepareUnlockedTestEntry(m_keyPair.publicKey(), randomPrivate, encryptionMutationFlag);
				return entryToMessage(entry);
			}

			auto prepareMessages(const std::vector<Key>& privateKeys, std::initializer_list<Sizes> messageDeltas) {
				if (messageDeltas.size() != privateKeys.size())
					CATAPULT_THROW_INVALID_ARGUMENT("arguments sizes mismatch");

				std::vector<std::vector<uint8_t>> messages;
				auto i = 0u;
				for (auto messageDelta : messageDeltas) {
					auto buffer = prepareMessage(privateKeys[i], test::EncryptionMutationFlag::None);
					if (Sizes::Underflow == messageDelta)
						buffer.resize(buffer.size() - 1);
					else if (Sizes::Overflow == messageDelta)
						buffer.resize(buffer.size() + 1);

					messages.push_back(buffer);
					++i;
				}

				return messages;
			}

			auto prepareInvalidMessage(const Key& randomPrivate, InvalidMessageFlag invalidMessageFlag) {
				if (InvalidMessageFlag::Invalid_Padding == invalidMessageFlag)
					return prepareMessage(randomPrivate, test::EncryptionMutationFlag::Mutate_Padding);

				std::vector<uint8_t> randomPrivateBuffer(randomPrivate.cbegin(), randomPrivate.cend());
				randomPrivateBuffer.resize(randomPrivateBuffer.size() + 1);
				auto entry = test::PrepareUnlockedTestEntry(m_keyPair.publicKey(), randomPrivateBuffer);
				return entryToMessage(entry);
			}

		private:
			std::vector<uint8_t> entryToMessage(const test::UnlockedTestEntry& entry) {
				std::vector<uint8_t> buffer;
				buffer.push_back(test::RandomByte() % 2);
				auto entryBuffer = test::ConvertUnlockedTestEntryToBuffer(entry);
				buffer.insert(buffer.end(), entryBuffer.cbegin(), entryBuffer.cend());
				return buffer;
			}

			void assertForwardedMessages(
					const std::vector<std::vector<uint8_t>>& collectedMessages,
					const std::vector<std::vector<uint8_t>>& messages,
					std::initializer_list<size_t> validIndexes) {
				ASSERT_EQ(collectedMessages.size(), validIndexes.size());
				auto iter = collectedMessages.cbegin();
				for (const auto index : validIndexes) {
					EXPECT_EQ(messages[index], *iter);
					++iter;
				}
			}

			void assertKeyPairs(
					const std::vector<crypto::KeyPair>& collectedKeyPairs,
					const std::vector<Key>& keys,
					std::initializer_list<size_t> validIndexes) {
				ASSERT_EQ(collectedKeyPairs.size(), validIndexes.size());
				auto iter = collectedKeyPairs.cbegin();
				for (const auto index : validIndexes) {
					auto expectedKeyPair = test::KeyPairFromPrivateKeyBuffer(keys[index]);
					EXPECT_EQ(expectedKeyPair.publicKey(), iter->publicKey());
					EXPECT_EQ(expectedKeyPair.privateKey(), iter->privateKey());
					++iter;
				}
			}

		private:
			test::TempDirectoryGuard m_directoryGuard;
			crypto::KeyPair m_keyPair;
		};

		// endregion

		void AssertConsumerFiltersMessages(
				std::initializer_list<size_t> validIndexes,
				std::initializer_list<TestContext::Sizes> messageDeltas) {
			// Arrange:
			TestContext context;
			auto keys = GenerateKeys(messageDeltas.size());
			auto messages = context.prepareMessages(keys, messageDeltas);
			context.write(messages);

			// Act + Assert:
			context.runConsumerAndAssert(keys, messages, validIndexes);
		}
	}

	// region consumer tests

	TEST(TEST_CLASS, ConsumerDoesNotFilterValidMessages) {
		AssertConsumerFiltersMessages({ 0, 1, 2 }, { TestContext::Sizes::Normal, TestContext::Sizes::Normal, TestContext::Sizes::Normal });
	}

	TEST(TEST_CLASS, ConsumerFiltersOutTooShortMessages) {
		AssertConsumerFiltersMessages({ 0, 2 }, { TestContext::Sizes::Normal, TestContext::Sizes::Underflow, TestContext::Sizes::Normal });
	}

	TEST(TEST_CLASS, ConsumerFiltersOutTooLongMessages) {
		AssertConsumerFiltersMessages({ 0, 2 }, { TestContext::Sizes::Normal, TestContext::Sizes::Overflow, TestContext::Sizes::Normal });
	}

	namespace {
		void AssertConsumerFiltersMessages(TestContext::InvalidMessageFlag invalidMessageFlag) {
			// Arrange:
			TestContext context;
			auto keys = GenerateKeys(3);
			std::vector<std::vector<uint8_t>> messages;
			messages.emplace_back(context.prepareMessage(keys[0], test::EncryptionMutationFlag::None));
			messages.emplace_back(context.prepareInvalidMessage(keys[1], invalidMessageFlag));
			messages.emplace_back(context.prepareMessage(keys[2], test::EncryptionMutationFlag::None));
			context.write(messages);

			// Act + Assert:
			context.runConsumerAndAssert(keys, messages, { 0, 2 });
		}
	}

	TEST(TEST_CLASS, ConsumerFiltersMessagesThatCannotBeDecrypted) {
		AssertConsumerFiltersMessages(TestContext::InvalidMessageFlag::Invalid_Padding);
	}

	TEST(TEST_CLASS, ConsumerFiltersMessagesThatHaveDecrytedDataOfInvalidSize) {
		AssertConsumerFiltersMessages(TestContext::InvalidMessageFlag::Invalid_Decrypted_Data_Size);
	}

	// endregion
}}
