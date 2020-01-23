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

#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionStatus.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

#define TEST_CLASS BatchSignatureConsumerTests // used to generate unique function names in macros
#define BLOCK_TEST_CLASS BlockBatchSignatureConsumerTests
#define TRANSACTION_TEST_CLASS TransactionBatchSignatureConsumerTests

	namespace {
		// region NotificationDescriptor

		enum class NotificationDescriptor {
			None = 0,
			Signature = 1,
			Verifiable = 2,
			Replay = 4
		};

		constexpr NotificationDescriptor operator|(NotificationDescriptor lhs, NotificationDescriptor rhs) {
			return static_cast<NotificationDescriptor>(utils::to_underlying_type(lhs) | utils::to_underlying_type(rhs));
		}

		constexpr bool HasFlag(NotificationDescriptor testedFlag, NotificationDescriptor value) {
			return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value));
		}

		void StripVerifiable(NotificationDescriptor& descriptor) {
			auto rawDescriptor = utils::to_underlying_type(descriptor) & ~utils::to_underlying_type(NotificationDescriptor::Verifiable);
			descriptor = static_cast<NotificationDescriptor>(rawDescriptor);
		}

		std::vector<NotificationDescriptor> GetMixedDescriptors() {
			auto descriptor1 = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable;
			auto descriptor2 = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable | NotificationDescriptor::Replay;
			auto descriptor3 = NotificationDescriptor::None;
			return { descriptor1, descriptor2, descriptor3, descriptor2, descriptor1, descriptor2, descriptor3 };
		}

		// endregion

		// region MockSignatureNotificationPublisher

		struct SignatureInput {
		public:
			SignatureInput()
					: Signer(crypto::KeyPair::FromPrivate(test::GenerateRandomPrivateKey()))
					, Signature(test::GenerateRandomByteArray<catapult::Signature>())
					, Data(test::GenerateRandomVector(256))
			{}

		public:
			crypto::KeyPair Signer;
			catapult::Signature Signature;
			std::vector<uint8_t> Data;
		};

		class MockSignatureNotificationPublisher : public model::NotificationPublisher {
		public:
			MockSignatureNotificationPublisher(
					const GenerationHash& generationHash,
					const std::vector<NotificationDescriptor>& descriptors,
					const std::unordered_set<size_t>& alwaysVerifiableIndexes)
					: m_generationHash(generationHash)
					, m_descriptors(descriptors)
					, m_alwaysVerifiableIndexes(alwaysVerifiableIndexes)
			{}

		public:
			const auto& entityInfos() const {
				return m_entityInfos;
			}

		public:
			void publish(const model::WeakEntityInfo& entityInfo, model::NotificationSubscriber& sub) const override {
				auto isAlwaysVerifiable = m_alwaysVerifiableIndexes.cend() != m_alwaysVerifiableIndexes.find(m_entityInfos.size());
				m_entityInfos.push_back(entityInfo);

				auto i = 0u;
				for (const auto& descriptor : m_descriptors) {
					m_signatureInputs.push_back(SignatureInput());
					auto& input = m_signatureInputs.back();

					if (!HasFlag(NotificationDescriptor::Signature, descriptor)) {
						sub.notify(model::AccountAddressNotification(test::GenerateRandomByteArray<UnresolvedAddress>()));
						continue;
					}

					auto replayProtectionMode = model::SignatureNotification::ReplayProtectionMode::Disabled;
					if (HasFlag(NotificationDescriptor::Replay, descriptor)) {
						crypto::Sign(input.Signer, { RawBuffer(m_generationHash), RawBuffer(input.Data) }, input.Signature);
						replayProtectionMode = model::SignatureNotification::ReplayProtectionMode::Enabled;
					} else {
						crypto::Sign(input.Signer, input.Data, input.Signature);
					}

					if (!HasFlag(NotificationDescriptor::Verifiable, descriptor) && !isAlwaysVerifiable)
						input.Signature[15] ^= 0xFF;

					const auto& signerPublicKey = input.Signer.publicKey();
					sub.notify(model::SignatureNotification(signerPublicKey, input.Signature, input.Data, replayProtectionMode));
					++i;
				}
			}

		private:
			const GenerationHash& m_generationHash;
			std::vector<NotificationDescriptor> m_descriptors;
			std::unordered_set<size_t> m_alwaysVerifiableIndexes;
			mutable model::WeakEntityInfos m_entityInfos;

			// backing for data stored by reference in SignatureNotification (for test purposes, only sign hashes)
			// use list so that tests will work with arbitrary number of elements without requiring preallocation
			mutable std::list<SignatureInput> m_signatureInputs;
		};

		// endregion

		// region BlockTraits

		constexpr bool RequiresAllPredicate(model::BasicEntityType, Timestamp, const Hash256&) {
			return true;
		}

		crypto::RandomFiller CreateRandomFiller() {
			return [](auto* pOut, auto count) {
				// can use low entropy source for tests
				utils::LowEntropyRandomGenerator().fill(pOut, count);
			};
		}

		struct BlockTraits {
		public:
			struct TestContext {
			public:
				explicit TestContext(
						const std::vector<NotificationDescriptor>& descriptors,
						const std::unordered_set<size_t>& alwaysVerifiableIndexes = {},
						const RequiresValidationPredicate& requiresValidationPredicate = RequiresAllPredicate)
						: GenerationHash(test::GenerateRandomByteArray<catapult::GenerationHash>())
						, pPublisher(std::make_shared<MockSignatureNotificationPublisher>(
								GenerationHash,
								descriptors,
								alwaysVerifiableIndexes))
						, pPool(test::CreateStartedIoThreadPool())
						, Consumer(CreateBlockBatchSignatureConsumer(
								GenerationHash,
								CreateRandomFiller(),
								pPublisher,
								pPool,
								requiresValidationPredicate))
				{}

			public:
				catapult::GenerationHash GenerationHash;
				std::shared_ptr<MockSignatureNotificationPublisher> pPublisher;
				std::shared_ptr<thread::IoThreadPool> pPool;

				disruptor::ConstBlockConsumer Consumer;
			};

		public:
			static auto CreateMultipleEntityElements() {
				auto pBlock1 = test::GenerateBlockWithTransactions(1, Height(246));
				auto pBlock2 = test::GenerateBlockWithTransactions(0, Height(247));
				auto pBlock3 = test::GenerateBlockWithTransactions(3, Height(248));
				auto pBlock4 = test::GenerateBlockWithTransactions(2, Height(249));
				return test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get(), pBlock4.get() });
			}

		public:
			static void AssertEntities(
					const disruptor::BlockElements& elements,
					const model::WeakEntityInfos& entityInfos,
					size_t numExpectedEntities,
					const RequiresValidationPredicate& requiresValidationPredicate) {
				// Arrange:
				model::WeakEntityInfos expectedEntityInfos;
				ExtractMatchingEntityInfos(elements, expectedEntityInfos, requiresValidationPredicate);

				// Assert:
				EXPECT_EQ(numExpectedEntities, entityInfos.size());
				EXPECT_EQ(expectedEntityInfos.size(), entityInfos.size());
				EXPECT_EQ(expectedEntityInfos, entityInfos);
			}

			static void AssertAllSignaturesVerify(const std::vector<NotificationDescriptor>& descriptors) {
				// Arrange:
				auto elements = CreateMultipleEntityElements();
				TestContext context(descriptors);

				// Act:
				auto result = context.Consumer(elements);

				// Assert:
				test::AssertContinued(result);
				AssertEntities(elements, context.pPublisher->entityInfos(), 10, RequiresAllPredicate);
			}

			static void AssertSomeSignaturesVerify(std::vector<NotificationDescriptor>&& descriptors) {
				// Arrange:
				auto elements = CreateMultipleEntityElements();
				StripVerifiable(descriptors[0]);
				StripVerifiable(descriptors[3]);
				TestContext context(descriptors, { 0, 2, 4 });

				// Act:
				auto result = context.Consumer(elements);

				// Assert:
				test::AssertAborted(result, Failure_Consumer_Batch_Signature_Not_Verifiable, disruptor::ConsumerResultSeverity::Fatal);
			}

			static void AssertNoSignaturesVerify(std::vector<NotificationDescriptor>&& descriptors) {
				// Arrange:
				auto elements = CreateMultipleEntityElements();
				StripVerifiable(descriptors[0]);
				StripVerifiable(descriptors[3]);
				TestContext context(descriptors);

				// Act:
				auto result = context.Consumer(elements);

				// Assert:
				test::AssertAborted(result, Failure_Consumer_Batch_Signature_Not_Verifiable, disruptor::ConsumerResultSeverity::Fatal);
			}
		};

		// endregion

		// region TransactionTraits

		model::WeakEntityInfos FilterEntityInfos(const TransactionElements& elements, const std::set<size_t>& indexes) {
			auto index = 0u;
			model::WeakEntityInfos entityInfos;
			for (const auto& element : elements) {
				if (indexes.cend() != indexes.find(index++))
					entityInfos.emplace_back(element.Transaction, element.EntityHash);
			}

			return entityInfos;
		}

		struct TransactionTraits {
		public:
			struct TestContext {
			public:
				explicit TestContext(
						const std::vector<NotificationDescriptor>& descriptors,
						const std::unordered_set<size_t>& alwaysVerifiableIndexes = {})
						: GenerationHash(test::GenerateRandomByteArray<catapult::GenerationHash>())
						, pPublisher(std::make_shared<MockSignatureNotificationPublisher>(
								GenerationHash,
								descriptors,
								alwaysVerifiableIndexes))
						, pPool(test::CreateStartedIoThreadPool())
						, Consumer(CreateTransactionBatchSignatureConsumer(
								GenerationHash,
								CreateRandomFiller(),
								pPublisher,
								pPool,
								[this](const auto& transaction, const auto& hash, auto result) {
									// notice that transaction.Deadline is used as transaction marker
									FailedTransactionStatuses.emplace_back(hash, transaction.Deadline, utils::to_underlying_type(result));
								}))
				{}

			public:
				catapult::GenerationHash GenerationHash;
				std::shared_ptr<MockSignatureNotificationPublisher> pPublisher;
				std::shared_ptr<thread::IoThreadPool> pPool;

				std::vector<model::TransactionStatus> FailedTransactionStatuses;
				disruptor::TransactionConsumer Consumer;
			};

		public:
			static auto CreateMultipleEntityElements() {
				auto pTransaction1 = test::GenerateRandomTransaction();
				auto pTransaction2 = test::GenerateRandomTransaction();
				auto pTransaction3 = test::GenerateRandomTransaction();
				auto pTransaction4 = test::GenerateRandomTransaction();
				return test::CreateTransactionElements({
					pTransaction1.get(), pTransaction2.get(), pTransaction3.get(), pTransaction4.get()
				});
			}

		public:
			static void AssertEntities(
					const model::WeakEntityInfos& expectedEntityInfos,
					const model::WeakEntityInfos& entityInfos,
					size_t numExpectedEntities) {
				EXPECT_EQ(numExpectedEntities, entityInfos.size());
				EXPECT_EQ(expectedEntityInfos.size(), entityInfos.size());
				EXPECT_EQ(expectedEntityInfos, entityInfos);
			}

			static void AssertAllSignaturesVerify(const std::vector<NotificationDescriptor>& descriptors) {
				// Arrange:
				auto elements = CreateMultipleEntityElements();
				TestContext context(descriptors);

				// Act:
				auto result = context.Consumer(elements);

				// Assert:
				test::AssertContinued(result);
				AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPublisher->entityInfos(), 4);

				// - no elements should have failed
				EXPECT_TRUE(context.FailedTransactionStatuses.empty());

				AssertPassed(elements[0], 0);
				AssertPassed(elements[1], 1);
				AssertPassed(elements[2], 2);
				AssertPassed(elements[3], 3);
			}

			static void AssertSomeSignaturesVerify(std::vector<NotificationDescriptor>&& descriptors) {
				// Arrange:
				auto elements = CreateMultipleEntityElements();
				StripVerifiable(descriptors[0]);
				StripVerifiable(descriptors[3]);
				TestContext context(descriptors, { 0, 2, 4 });

				// Act:
				auto result = context.Consumer(elements);

				// Assert:
				test::AssertAborted(result, Failure_Consumer_Batch_Signature_Not_Verifiable, disruptor::ConsumerResultSeverity::Fatal);
				AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPublisher->entityInfos(), 4);

				// - two elements should have failed
				ASSERT_EQ(2u, context.FailedTransactionStatuses.size());

				AssertPassed(elements[0], 0);
				AssertFailed(elements[1], context.FailedTransactionStatuses[0], 1);
				AssertPassed(elements[2], 2);
				AssertFailed(elements[3], context.FailedTransactionStatuses[1], 3);
			}

			static void AssertNoSignaturesVerify(std::vector<NotificationDescriptor>&& descriptors) {
				// Arrange:
				auto elements = CreateMultipleEntityElements();
				StripVerifiable(descriptors[0]);
				StripVerifiable(descriptors[3]);
				TestContext context(descriptors);

				// Act:
				auto result = context.Consumer(elements);

				// Assert:
				test::AssertAborted(result, Failure_Consumer_Batch_Signature_Not_Verifiable, disruptor::ConsumerResultSeverity::Fatal);
				AssertEntities(FilterEntityInfos(elements, { 0, 1, 2, 3 }), context.pPublisher->entityInfos(), 4);

				// - all elements should have failed
				ASSERT_EQ(4u, context.FailedTransactionStatuses.size());

				AssertFailed(elements[0], context.FailedTransactionStatuses[0], 0);
				AssertFailed(elements[1], context.FailedTransactionStatuses[1], 1);
				AssertFailed(elements[2], context.FailedTransactionStatuses[2], 2);
				AssertFailed(elements[3], context.FailedTransactionStatuses[3], 3);
			}

		private:
			static void AssertPassed(const disruptor::FreeTransactionElement& transactionElement, size_t index) {
				EXPECT_EQ(disruptor::ConsumerResultSeverity::Success, transactionElement.ResultSeverity) << "element at " << index;
			}

			static void AssertFailed(
					const disruptor::FreeTransactionElement& transactionElement,
					const model::TransactionStatus& transactionStatus,
					size_t index) {
				EXPECT_EQ(disruptor::ConsumerResultSeverity::Failure, transactionElement.ResultSeverity) << "element at " << index;

				EXPECT_EQ(transactionElement.EntityHash, transactionStatus.Hash) << "element at " << index;
				EXPECT_EQ(
						Failure_Consumer_Batch_Signature_Not_Verifiable,
						static_cast<validators::ValidationResult>(transactionStatus.Status)) << "element at " << index;
			}
		};

		// endregion
	}

#define ALL_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(BLOCK_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockTraits>(); } \
	TEST(TRANSACTION_TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region all - no signatures

	ALL_TEST(CanProcessZeroEntities) {
		// Arrange:
		typename TTraits::TestContext context({});

		// Assert:
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	ALL_TEST(CanProcessEntitiesWithNoSignatureNotifications) {
		auto descriptor = NotificationDescriptor::None;
		TTraits::AssertAllSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	// endregion

	// region all - signatures (success)

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_AllVerifiable_NoReplay) {
		auto descriptor = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable;
		TTraits::AssertAllSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_AllVerifiable_Replay) {
		auto descriptor = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable | NotificationDescriptor::Replay;
		TTraits::AssertAllSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_AllVerifiable_Mixed) {
		TTraits::AssertAllSignaturesVerify(GetMixedDescriptors());
	}

	// endregion

	// region all - signatures (failure)

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_SomeVerifiable_NoReplay) {
		auto descriptor = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable;
		TTraits::AssertSomeSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_SomeVerifiable_Replay) {
		auto descriptor = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable | NotificationDescriptor::Replay;
		TTraits::AssertSomeSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_SomeVerifiable_Mixed) {
		TTraits::AssertSomeSignaturesVerify(GetMixedDescriptors());
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_NoneVerifiable_NoReplay) {
		auto descriptor = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable;
		TTraits::AssertNoSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_NoneVerifiable_Replay) {
		auto descriptor = NotificationDescriptor::Signature | NotificationDescriptor::Verifiable | NotificationDescriptor::Replay;
		TTraits::AssertNoSignaturesVerify(std::vector<NotificationDescriptor>(7, descriptor));
	}

	ALL_TEST(CanProcessEntitiesWithSignatureNotifications_NoneVerifiable_Mixed) {
		TTraits::AssertNoSignaturesVerify(GetMixedDescriptors());
	}

	// endregion

	// region block only

	TEST(BLOCK_TEST_CLASS, CanProcessEntitiesWithSignatureNotifications_AllVerifiable_Mixed_NotAllRequired) {
		// Arrange:
		auto elements = BlockTraits::CreateMultipleEntityElements();
		auto requiresValidationPredicate = [&elements](auto, auto, const auto& hash) {
			return elements[1].EntityHash != hash && elements[2].EntityHash != hash;
		};

		BlockTraits::TestContext context(GetMixedDescriptors(), {}, requiresValidationPredicate);

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		BlockTraits::AssertEntities(elements, context.pPublisher->entityInfos(), 8, requiresValidationPredicate);
	}

	// endregion

	// region transaction only

	TEST(TRANSACTION_TEST_CLASS, CanProcessEntitiesWithSignatureNotifications_AllVerifiable_Mixed_NotAllRequired) {
		// Arrange:
		auto elements = TransactionTraits::CreateMultipleEntityElements();
		elements[0].ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;
		elements[2].ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;
		TransactionTraits::TestContext context(GetMixedDescriptors());

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		TransactionTraits::AssertEntities(FilterEntityInfos(elements, { 1, 3 }), context.pPublisher->entityInfos(), 2);

		// - no elements should have failed
		EXPECT_TRUE(context.FailedTransactionStatuses.empty());
		EXPECT_EQ(disruptor::ConsumerResultSeverity::Neutral, elements[0].ResultSeverity);
		EXPECT_EQ(disruptor::ConsumerResultSeverity::Success, elements[1].ResultSeverity);
		EXPECT_EQ(disruptor::ConsumerResultSeverity::Neutral, elements[2].ResultSeverity);
		EXPECT_EQ(disruptor::ConsumerResultSeverity::Success, elements[3].ResultSeverity);
	}

	// endregion
}}
