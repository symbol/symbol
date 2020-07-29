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
#include "harvesting/tests/test/HarvestRequestEncryptedPayload.h"
#include "tests/test/crypto/EncryptionTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS UnlockedFileQueueConsumerTests

	// region TryDecryptBlockGeneratorAccountDescriptor

	namespace {
		constexpr auto Encrypted_Data_Size = 2 * Key::Size;

		void AssertFailureWhenDecryptedDataHasInvalidSize(size_t dataSize) {
			// Arrange:
			auto clearText = test::GenerateRandomVector(dataSize);
			auto recipientKeyPair = test::GenerateKeyPair();
			auto publicKeyPrefixedEncryptedPayload = test::GenerateEphemeralAndEncrypt(clearText, recipientKeyPair.publicKey());

			// Act:
			auto decryptedPair = TryDecryptBlockGeneratorAccountDescriptor(publicKeyPrefixedEncryptedPayload, recipientKeyPair);

			// Assert:
			EXPECT_FALSE(decryptedPair.second);
		}
	}

	TEST(TEST_CLASS, TryDecryptBlockGeneratorAccountDescriptor_FailsWhenDecryptedDataIsTooSmall) {
		AssertFailureWhenDecryptedDataHasInvalidSize(Encrypted_Data_Size - 1);
	}

	TEST(TEST_CLASS, TryDecryptBlockGeneratorAccountDescriptor_FailsWhenDecryptedDataIsTooLarge) {
		AssertFailureWhenDecryptedDataHasInvalidSize(Encrypted_Data_Size + 1);
	}

	TEST(TEST_CLASS, TryDecryptBlockGeneratorAccountDescriptor_SucceedsWhenDecryptedDataHasProperSize) {
		// Arrange:
		auto clearText = test::GenerateRandomVector(Encrypted_Data_Size);
		auto recipientKeyPair = test::GenerateKeyPair();
		auto publicKeyPrefixedEncryptedPayload = test::GenerateEphemeralAndEncrypt(clearText, recipientKeyPair.publicKey());

		// Act:
		auto decryptedPair = TryDecryptBlockGeneratorAccountDescriptor(publicKeyPrefixedEncryptedPayload, recipientKeyPair);

		// Assert:
		ASSERT_TRUE(decryptedPair.second);
		EXPECT_EQ_MEMORY(clearText.data(), decryptedPair.first.signingKeyPair().privateKey().data(), Key::Size);
		EXPECT_EQ_MEMORY(clearText.data() + Key::Size, decryptedPair.first.vrfKeyPair().privateKey().data(), Key::Size);
	}

	// endregion

	namespace {
		// region utils

		auto SerializeHarvestRequest(const HarvestRequest& request) {
			std::vector<uint8_t> requestBuffer(1 + Key::Size + request.EncryptedPayload.Size);
			requestBuffer[0] = static_cast<uint8_t>(request.Operation);
			reinterpret_cast<Key&>(requestBuffer[1]) = request.MainAccountPublicKey;
			std::memcpy(&requestBuffer[1 + Key::Size], request.EncryptedPayload.pData, request.EncryptedPayload.Size);
			return requestBuffer;
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
					const std::vector<BlockGeneratorAccountDescriptor>& descriptors,
					const std::vector<std::vector<uint8_t>>& messages,
					std::initializer_list<size_t> validIndexes) {
				// Act:
				config::CatapultDirectory directory(m_directoryGuard.name());
				std::vector<std::vector<uint8_t>> collectedMessages;
				std::vector<BlockGeneratorAccountDescriptor> collectedDescriptors;
				UnlockedFileQueueConsumer(directory, m_keyPair, [&collectedMessages, &collectedDescriptors](
						const auto& request,
						auto&& descriptor) {
					collectedMessages.emplace_back(SerializeHarvestRequest(request));
					collectedDescriptors.emplace_back(std::move(descriptor));
				});

				// Assert:
				AssertForwarded<std::vector<uint8_t>>(messages, collectedMessages, validIndexes, [](
						const auto& expected,
						const auto& actual,
						auto index) {
					EXPECT_EQ(expected, actual) << "message at " << index;
				});
				AssertForwarded<BlockGeneratorAccountDescriptor>(descriptors, collectedDescriptors, validIndexes, [](
						const auto& expected,
						const auto& actual,
						auto index) {
					EXPECT_EQ(expected.signingKeyPair().publicKey(), actual.signingKeyPair().publicKey()) << "descriptor at " << index;
					EXPECT_EQ(expected.vrfKeyPair().publicKey(), actual.vrfKeyPair().publicKey()) << "descriptor at " << index;
				});
			}

		public:
			enum class InvalidMessageFlag { Invalid_Tag, Invalid_Decrypted_Data_Size };
			enum class Sizes { Underflow, Normal, Overflow };

			auto prepareMessage(const BlockGeneratorAccountDescriptor& descriptor) {
				auto clearText = test::ToClearTextBuffer(descriptor);
				auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(m_keyPair.publicKey(), clearText);
				return EncryptedPayloadToMessage(encryptedPayload);
			}

			auto prepareMessages(
					const std::vector<BlockGeneratorAccountDescriptor>& descriptors,
					std::initializer_list<Sizes> messageDeltas) {
				if (messageDeltas.size() != descriptors.size())
					CATAPULT_THROW_INVALID_ARGUMENT("arguments sizes mismatch");

				std::vector<std::vector<uint8_t>> messages;
				auto i = 0u;
				for (auto messageDelta : messageDeltas) {
					auto buffer = prepareMessage(descriptors[i]);
					if (Sizes::Underflow == messageDelta)
						buffer.resize(buffer.size() - 1);
					else if (Sizes::Overflow == messageDelta)
						buffer.resize(buffer.size() + 1);

					messages.push_back(buffer);
					++i;
				}

				return messages;
			}

			auto prepareInvalidMessage(const BlockGeneratorAccountDescriptor& descriptor, InvalidMessageFlag invalidMessageFlag) {
				if (InvalidMessageFlag::Invalid_Tag == invalidMessageFlag) {
					auto messageBuffer = prepareMessage(descriptor);
					messageBuffer[Key::Size + 1] ^= 0xFF;
					return messageBuffer;
				}

				auto clearText = test::ToClearTextBuffer(descriptor);
				clearText.resize(clearText.size() - 1);
				auto encryptedPayload = test::PrepareHarvestRequestEncryptedPayload(m_keyPair.publicKey(), clearText);
				return EncryptedPayloadToMessage(encryptedPayload);
			}

		private:
			static std::vector<uint8_t> EncryptedPayloadToMessage(const test::HarvestRequestEncryptedPayload& encryptedPayload) {
				std::vector<uint8_t> messageBuffer;
				messageBuffer.push_back(test::RandomByte() % 2);

				auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
				messageBuffer.insert(messageBuffer.end(), mainAccountPublicKey.cbegin(), mainAccountPublicKey.cend());

				auto encryptedBuffer = test::CopyHarvestRequestEncryptedPayloadToBuffer(encryptedPayload);
				messageBuffer.insert(messageBuffer.end(), encryptedBuffer.cbegin(), encryptedBuffer.cend());
				return messageBuffer;
			}

			template<typename T>
			static void AssertForwarded(
					const std::vector<T>& allItems,
					const std::vector<T>& collectedItems,
					std::initializer_list<size_t> validIndexes,
					const consumer<const T&, const T&, size_t>& checkEquality) {
				ASSERT_EQ(validIndexes.size(), collectedItems.size());

				auto iter = collectedItems.cbegin();
				for (const auto index : validIndexes) {
					checkEquality(allItems[index], *iter, index);
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
			auto descriptors = test::GenerateRandomAccountDescriptors(messageDeltas.size());
			auto messages = context.prepareMessages(descriptors, messageDeltas);
			context.write(messages);

			// Act + Assert:
			context.runConsumerAndAssert(descriptors, messages, validIndexes);
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
			auto descriptors = test::GenerateRandomAccountDescriptors(3);
			std::vector<std::vector<uint8_t>> messages;
			messages.emplace_back(context.prepareMessage(descriptors[0]));
			messages.emplace_back(context.prepareInvalidMessage(descriptors[1], invalidMessageFlag));
			messages.emplace_back(context.prepareMessage(descriptors[2]));
			context.write(messages);

			// Act + Assert:
			context.runConsumerAndAssert(descriptors, messages, { 0, 2 });
		}
	}

	TEST(TEST_CLASS, ConsumerFiltersMessagesThatCannotBeDecrypted) {
		AssertConsumerFiltersMessages(TestContext::InvalidMessageFlag::Invalid_Tag);
	}

	TEST(TEST_CLASS, ConsumerFiltersMessagesThatHaveDecryptedDataOfInvalidSize) {
		AssertConsumerFiltersMessages(TestContext::InvalidMessageFlag::Invalid_Decrypted_Data_Size);
	}

	// endregion
}}
