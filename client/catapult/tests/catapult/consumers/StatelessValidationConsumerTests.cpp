#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/InputUtils.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "tests/catapult/consumers/utils/ConsumerTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/other/mocks/MockDispatchFunctions.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;

namespace catapult { namespace consumers {

	namespace {
		constexpr bool RequiresAllPredicate(model::BasicEntityType, Timestamp, const Hash256&) {
			return true;
		}

		struct BlockTestContext {
		public:
			explicit BlockTestContext(const RequiresValidationPredicate& requiresValidationPredicate = RequiresAllPredicate)
					: pValidator(std::make_shared<stateless::AggregateEntityValidator>(ValidatorVectorT<>()))
					, Consumer(CreateBlockStatelessValidationConsumer(
							pValidator,
							mocks::Wrap(DispatchFunc),
							requiresValidationPredicate))
			{}

		public:
			std::shared_ptr<stateless::AggregateEntityValidator> pValidator;
			mocks::MockParallelDispatchFunc DispatchFunc;
			disruptor::ConstBlockConsumer Consumer;
		};

		struct BlockTraits {
			using TestContextType = BlockTestContext;

			constexpr static auto Num_Sub_Entities_Single = 4u;
			constexpr static auto Num_Sub_Entities_Multiple = 10u;

			static auto CreateSingleEntityElements() {
				auto pBlock = test::GenerateBlockWithTransactionsAtHeight(3, 246);
				return test::CreateBlockElements({ pBlock.get() });
			}

			static auto CreateMultipleEntityElements() {
				auto pBlock1 = test::GenerateBlockWithTransactionsAtHeight(1, 246);
				auto pBlock2 = test::GenerateBlockWithTransactionsAtHeight(0, 247);
				auto pBlock3 = test::GenerateBlockWithTransactionsAtHeight(3, 248);
				auto pBlock4 = test::GenerateBlockWithTransactionsAtHeight(2, 249);
				return test::CreateBlockElements({ pBlock1.get(), pBlock2.get(), pBlock3.get(), pBlock4.get() });
			}

			static void AssertEntities(
					const disruptor::BlockElements& elements,
					const std::vector<mocks::DispatchParams>& params,
					size_t numExpectedEntities,
					const RequiresValidationPredicate& requiresValidationPredicate = RequiresAllPredicate) {
				// Arrange:
				model::WeakEntityInfos expectedEntityInfos;
				ExtractMatchingEntityInfos(elements, expectedEntityInfos, requiresValidationPredicate);

				// Assert:
				ASSERT_EQ(1u, params.size());
				EXPECT_EQ(numExpectedEntities, params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos.size(), params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos, params[0].EntityInfos);
			}
		};

		struct TransactionTestContext {
		public:
			TransactionTestContext()
					: pValidator(std::make_shared<stateless::AggregateEntityValidator>(ValidatorVectorT<>()))
					, Consumer(CreateTransactionStatelessValidationConsumer(pValidator, mocks::Wrap(DispatchFunc)))
			{}

		public:
			std::shared_ptr<stateless::AggregateEntityValidator> pValidator;
			mocks::MockParallelDispatchFunc DispatchFunc;
			disruptor::ConstTransactionConsumer Consumer;
		};

		struct TransactionTraits {
			using TestContextType = TransactionTestContext;

			constexpr static auto Num_Sub_Entities_Single = 1u;
			constexpr static auto Num_Sub_Entities_Multiple = 4u;

			static auto CreateSingleEntityElements() {
				auto pTransaction = test::GenerateRandomTransaction();
				return test::CreateTransactionElements({ pTransaction.get() });
			}

			static auto CreateMultipleEntityElements() {
				auto pTransaction1 = test::GenerateRandomTransaction();
				auto pTransaction2 = test::GenerateRandomTransaction();
				auto pTransaction3 = test::GenerateRandomTransaction();
				auto pTransaction4 = test::GenerateRandomTransaction();
				return test::CreateTransactionElements({
					pTransaction1.get(),
					pTransaction2.get(),
					pTransaction3.get(),
					pTransaction4.get()
				});
			}

			static void AssertEntities(
					const disruptor::TransactionElements& elements,
					const std::vector<mocks::DispatchParams>& params,
					size_t numExpectedEntities) {
				// Arrange:
				model::WeakEntityInfos expectedEntityInfos;
				ExtractEntityInfos(elements, expectedEntityInfos);

				// Assert:
				ASSERT_EQ(1u, params.size());
				EXPECT_EQ(numExpectedEntities, params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos.size(), params[0].EntityInfos.size());
				EXPECT_EQ(expectedEntityInfos, params[0].EntityInfos);
			}
		};
	}

#define TEST_PREFIX StatelessValidationConsumerTests
#define ENTITY_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_PREFIX, TEST_NAME)(); \
	TEST(BlockStatelessValidationConsumerTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_PREFIX, TEST_NAME)<BlockTraits>(); } \
	TEST(TransactionStatelessValidationConsumerTests, TEST_NAME) { TRAITS_TEST_NAME(TEST_PREFIX, TEST_NAME)<TransactionTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_PREFIX, TEST_NAME)()

	// region shared tests

	ENTITY_BASED_TEST(CanProcessZeroEntities) {
		// Assert:
		typename TTraits::TestContextType context;
		test::AssertPassthroughForEmptyInput(context.Consumer);
	}

	ENTITY_BASED_TEST(CanValidateSingleEntity) {
		// Arrange:
		typename TTraits::TestContextType context;
		auto elements = TTraits::CreateSingleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		TTraits::AssertEntities(elements, context.DispatchFunc.params(), TTraits::Num_Sub_Entities_Single);
	}

	ENTITY_BASED_TEST(CanValidateMultipleEntities) {
		// Arrange:
		typename TTraits::TestContextType context;
		auto elements = TTraits::CreateMultipleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		TTraits::AssertEntities(elements, context.DispatchFunc.params(), TTraits::Num_Sub_Entities_Multiple);
	}

	namespace {
		template<typename TTraits>
		void AssertSkipResult(ValidationResult validationResult) {
			// Arrange:
			typename TTraits::TestContextType context;
			auto elements = TTraits::CreateSingleEntityElements();

			context.DispatchFunc.setResult(validationResult);

			// Act:
			auto result = context.Consumer(elements);

			// Assert:
			test::AssertAborted(result, validationResult);
			TTraits::AssertEntities(elements, context.DispatchFunc.params(), TTraits::Num_Sub_Entities_Single);
		}
	}

	ENTITY_BASED_TEST(NeutralValidationResultIsMappedToSkipConsumerResult) {
		// Assert:
		AssertSkipResult<TTraits>(ValidationResult::Neutral);
	}

	ENTITY_BASED_TEST(FailureValidationResultIsMappedToSkipConsumerResult) {
		// Assert:
		AssertSkipResult<TTraits>(ValidationResult::Failure);
	}

	// endregion

	// region block tests

	TEST(BlockStatelessValidationConsumerTests, CanValidateEmptyBlock) {
		// Arrange:
		BlockTestContext context;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(0, 246);
		auto elements = test::CreateBlockElements({ pBlock.get() });

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);
		BlockTraits::AssertEntities(elements, context.DispatchFunc.params(), 1);
	}

	TEST(BlockStatelessValidationConsumerTests, CanValidateMultipleBlocksWithCustomPredicate) {
		// Arrange:
		auto predicateFactory = []() {
			auto pCounter = std::make_shared<size_t>(0u);
			return [pCounter](auto, auto, const auto&) { return 0 == (*pCounter)++ % 2; };
		};

		BlockTestContext context(predicateFactory());
		auto elements = BlockTraits::CreateMultipleEntityElements();

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);

		auto numExpectedEntities = BlockTraits::Num_Sub_Entities_Multiple / 2;
		BlockTraits::AssertEntities(elements, context.DispatchFunc.params(), numExpectedEntities, predicateFactory());
	}

	// endregion

	// region transaction tests

	TEST(TransactionStatelessValidationConsumerTests, CanValidateMultipleTransactionsWithSomeSkipped) {
		// Arrange:
		TransactionTestContext context;
		auto elements = TransactionTraits::CreateMultipleEntityElements();
		elements[1].Skip = true;

		// Act:
		auto result = context.Consumer(elements);

		// Assert:
		test::AssertContinued(result);

		auto numExpectedEntities = TransactionTraits::Num_Sub_Entities_Multiple - 1;
		TransactionTraits::AssertEntities(elements, context.DispatchFunc.params(), numExpectedEntities);
	}

	// endregion
}}
