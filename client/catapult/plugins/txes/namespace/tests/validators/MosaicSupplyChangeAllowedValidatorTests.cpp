#include "src/validators/Validators.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicSupplyChangeAllowedValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicSupplyChangeAllowed, Amount())

	namespace {
		constexpr auto Max_Divisible_Units = Amount(std::numeric_limits<Amount::ValueType>::max());

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				Height height,
				const model::MosaicSupplyChangeNotification& notification,
				Amount maxDivisibleUnits = Max_Divisible_Units) {
			// Arrange:
			auto pValidator = CreateMosaicSupplyChangeAllowedValidator(maxDivisibleUnits);

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "id " << notification.MosaicId << ", delta " << notification.Delta;
		}

		void AddMosaic(
				cache::CatapultCache& cache,
				MosaicId id,
				Amount mosaicSupply,
				const Key& owner,
				Amount ownerSupply,
				model::MosaicFlags flags = model::MosaicFlags::Supply_Mutable) {
			auto delta = cache.createDelta();

			// add a mosaic definition with the desired flags
			model::MosaicProperties::PropertyValuesContainer values{};
			values[utils::to_underlying_type(model::MosaicPropertyId::Flags)] = utils::to_underlying_type(flags);

			auto& mosaicCacheDelta = delta.sub<cache::MosaicCache>();
			auto definition = state::MosaicDefinition(Height(50), Key(), model::MosaicProperties::FromValues(values));
			auto entry = state::MosaicEntry(NamespaceId(11), id, definition);
			entry.increaseSupply(mosaicSupply);
			mosaicCacheDelta.insert(entry);

			test::AddMosaicOwner(delta, id, owner, ownerSupply);
			cache.commit(Height());
		}
	}

	// region immutable supply

	namespace {
		void AssertCanChangeImmutableSupplyWhenOwnerHasCompleteSupply(model::MosaicSupplyChangeDirection direction) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::MosaicSupplyChangeNotification(signer, MosaicId(123), direction, Amount(100));

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(500), signer, Amount(500), model::MosaicFlags::None);

			// Assert:
			AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanIncreaseImmutableSupplyWhenOwnerHasCompleteSupply) {
		// Assert:
		AssertCanChangeImmutableSupplyWhenOwnerHasCompleteSupply(model::MosaicSupplyChangeDirection::Increase);
	}

	TEST(TEST_CLASS, CanDecreaseImmutableSupplyWhenOwnerHasCompleteSupply) {
		// Assert:
		AssertCanChangeImmutableSupplyWhenOwnerHasCompleteSupply(model::MosaicSupplyChangeDirection::Decrease);
	}

	namespace {
		void AssertCannotChangeImmutableSupplyWhenOwnerHasPartialSupply(model::MosaicSupplyChangeDirection direction) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::MosaicSupplyChangeNotification(signer, MosaicId(123), direction, Amount(100));

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), Amount(500), signer, Amount(499), model::MosaicFlags::None);

			// Assert:
			AssertValidationResult(Failure_Mosaic_Supply_Immutable, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CannotIncreaseImmutableSupplyWhenOwnerHasPartialSupply) {
		// Assert:
		AssertCannotChangeImmutableSupplyWhenOwnerHasPartialSupply(model::MosaicSupplyChangeDirection::Increase);
	}

	TEST(TEST_CLASS, CannotDecreaseImmutableSupplyWhenOwnerHasPartialSupply) {
		// Assert:
		AssertCannotChangeImmutableSupplyWhenOwnerHasPartialSupply(model::MosaicSupplyChangeDirection::Decrease);
	}

	// endregion

	// region decrease supply

	namespace {
		void AssertDecreaseValidationResult(ValidationResult expectedResult, Amount mosaicSupply, Amount ownerSupply, Amount delta) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto direction = model::MosaicSupplyChangeDirection::Decrease;
			auto notification = model::MosaicSupplyChangeNotification(signer, MosaicId(123), direction, delta);

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), mosaicSupply, signer, ownerSupply);

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification);
		}
	}

	TEST(TEST_CLASS, CanDecreaseMutableSupplyByLessThanOwnerSupply) {
		// Assert:
		AssertDecreaseValidationResult(ValidationResult::Success, Amount(500), Amount(400), Amount(300));
		AssertDecreaseValidationResult(ValidationResult::Success, Amount(500), Amount(400), Amount(399));
	}

	TEST(TEST_CLASS, CanDecreaseMutableSupplyByEntireOwnerSupply) {
		// Assert:
		AssertDecreaseValidationResult(ValidationResult::Success, Amount(500), Amount(400), Amount(400));
	}

	TEST(TEST_CLASS, CannotDecreaseMutableSupplyByGreaterThanOwnerSupply) {
		// Assert:
		AssertDecreaseValidationResult(Failure_Mosaic_Supply_Negative, Amount(500), Amount(400), Amount(401));
		AssertDecreaseValidationResult(Failure_Mosaic_Supply_Negative, Amount(500), Amount(400), Amount(500));
	}

	// endregion

	// region increase

	namespace {
		void AssertIncreaseValidationResult(ValidationResult expectedResult, Amount maxDivisibleUnits, Amount mosaicSupply, Amount delta) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto direction = model::MosaicSupplyChangeDirection::Increase;
			auto notification = model::MosaicSupplyChangeNotification(signer, MosaicId(123), direction, delta);

			auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddMosaic(cache, MosaicId(123), mosaicSupply, signer, Amount(111));

			// Assert:
			AssertValidationResult(expectedResult, cache, Height(100), notification, maxDivisibleUnits);
		}
	}

	TEST(TEST_CLASS, CanIncreaseMutableSupplyToLessThanDivisibleUnits) {
		// Assert:
		AssertIncreaseValidationResult(ValidationResult::Success, Amount(900), Amount(500), Amount(300));
		AssertIncreaseValidationResult(ValidationResult::Success, Amount(900), Amount(500), Amount(399));
	}

	TEST(TEST_CLASS, CanIncreaseMutableSupplyToExactlyDivisibleUnits) {
		// Assert:
		AssertIncreaseValidationResult(ValidationResult::Success, Amount(900), Amount(500), Amount(400));
	}

	TEST(TEST_CLASS, CannotIncreaseMutableSupplyToGreaterThanDivisibleUnits) {
		// Assert:
		AssertIncreaseValidationResult(Failure_Mosaic_Supply_Exceeded, Amount(900), Amount(500), Amount(401));
		AssertIncreaseValidationResult(Failure_Mosaic_Supply_Exceeded, Amount(900), Amount(500), Amount(500));
	}

	TEST(TEST_CLASS, CannotIncreaseMutableSupplyIfOverflowIsDetected) {
		// Assert:
		AssertIncreaseValidationResult(Failure_Mosaic_Supply_Exceeded, Amount(900), Amount(500), Max_Divisible_Units);
	}

	// endregion
}}
