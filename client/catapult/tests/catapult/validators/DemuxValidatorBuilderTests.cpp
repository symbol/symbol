#include "catapult/validators/DemuxValidatorBuilder.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/catapult/validators/utils/AggregateValidatorTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS DemuxValidatorBuilderTests

	namespace {
		struct TestContext {
		public:
			std::vector<uint16_t> Breadcrumbs;
			std::unique_ptr<const stateful::AggregateNotificationValidator> pDemuxValidator;

		public:
			ValidationResult validate(uint8_t notificationId) {
				auto cache = test::CreateEmptyCatapultCache();
				auto cacheView = cache.createView();
				auto context = test::CreateValidatorContext(Height(123), cacheView.toReadOnly());
				return test::ValidateNotification<model::Notification>(
						*pDemuxValidator,
						test::TaggedNotification(notificationId),
						context);
			}
		};

		std::unique_ptr<TestContext> CreateTestContext(size_t numValidators, bool varyValidators = false) {
			auto pContext = std::make_unique<TestContext>();
			stateful::DemuxValidatorBuilder builder;
			for (auto i = 0u; i < numValidators; ++i) {
				auto id = static_cast<uint8_t>(i + 1);
				if (!varyValidators || 1 == id % 2)
					builder.add(mocks::CreateTaggedBreadcrumbValidator(id, pContext->Breadcrumbs));
				else
					builder.add(mocks::CreateTaggedBreadcrumbValidator2(id, pContext->Breadcrumbs));
			}

			auto pDemuxValidator = builder.build();
			pContext->pDemuxValidator = std::move(pDemuxValidator);
			return pContext;
		}
	}

	// region basic delegation

	TEST(TEST_CLASS, CanCreateEmptyDemuxValidator) {
		// Act:
		auto pContext = CreateTestContext(0);
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<std::string> expectedNames;
		EXPECT_EQ(0u, pContext->Breadcrumbs.size());
		EXPECT_EQ("{}", pContext->pDemuxValidator->name());
		EXPECT_EQ(expectedNames, pContext->pDemuxValidator->names());
	}

	TEST(TEST_CLASS, CanCreateDemuxValidatorWithMultipleValidators) {
		// Act:
		auto pContext = CreateTestContext(10);
		auto result = pContext->validate(12);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<std::string> expectedNames{ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
		EXPECT_EQ(10u, pContext->Breadcrumbs.size());
		EXPECT_EQ("{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }", pContext->pDemuxValidator->name());
		EXPECT_EQ(expectedNames, pContext->pDemuxValidator->names());
	}

	TEST(TEST_CLASS, AddAllowsChaining) {
		// Arrange:
		auto pContext = std::make_unique<TestContext>();
		stateful::DemuxValidatorBuilder builder;

		// Act:
		builder
			.add(mocks::CreateTaggedBreadcrumbValidator(2, pContext->Breadcrumbs))
			.add(mocks::CreateTaggedBreadcrumbValidator2(3, pContext->Breadcrumbs))
			.add(mocks::CreateTaggedBreadcrumbValidator(4, pContext->Breadcrumbs));
		pContext->pDemuxValidator = builder.build();

		// Act:
		auto result = pContext->validate(7);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0702, 0x0704 };
		EXPECT_EQ(2u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region validate

	TEST(TEST_CLASS, AggregateValidatorForwardsToAllValidatorsOnSuccess) {
		// Act:
		auto pContext = CreateTestContext(5, true);
		auto result = pContext->validate(4);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		std::vector<uint16_t> expectedBreadcrumbs{ 0x0401, 0x0403, 0x0405 };
		EXPECT_EQ(3u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	TEST(TEST_CLASS, AggregateValidatorCanPerformMultipleValidations) {
		// Act:
		auto pContext = CreateTestContext(3, true);
		pContext->validate(2);
		pContext->validate(7);
		pContext->validate(5);
		pContext->validate(1);

		// Assert:
		std::vector<uint16_t> expectedBreadcrumbs{
			0x0201, 0x0203,
			0x0701, 0x0703,
			0x0501, 0x0503,
			0x0101, 0x0103,
		};
		EXPECT_EQ(8u, pContext->Breadcrumbs.size());
		EXPECT_EQ(expectedBreadcrumbs, pContext->Breadcrumbs);
	}

	// endregion

	// region forwarding

	TEST(TEST_CLASS, NotificationsAreForwardedToChildValidators) {
		// Assert:
		AssertNotificationsAreForwardedToChildValidators(
				stateful::DemuxValidatorBuilder(),
				[](auto& builder, auto&& pValidator) {
					builder.add(std::move(pValidator));
				});
	}

	TEST(TEST_CLASS, ContextsAreForwardedToChildValidators) {
		// Assert:
		AssertContextsAreForwardedToChildValidators(
				stateful::DemuxValidatorBuilder(),
				[](auto& builder, auto&& pValidator) {
					builder.add(std::move(pValidator));
				});
	}

	// endregion

	// region filtering

	namespace {
		using Breadcrumbs = std::vector<std::string>;

		template<typename TNotification>
		class MockBreadcrumbValidator : public stateful::NotificationValidatorT<TNotification> {
		public:
			MockBreadcrumbValidator(const std::string& name, Breadcrumbs& breadcrumbs)
					: m_name(name)
					, m_breadcrumbs(breadcrumbs)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(const TNotification&, const ValidatorContext&) const override {
				m_breadcrumbs.push_back(m_name);
				return ValidationResult::Success;
			}

		private:
			std::string m_name;
			Breadcrumbs& m_breadcrumbs;
		};

		template<typename TNotification = model::Notification>
		stateful::NotificationValidatorPointerT<TNotification> CreateBreadcrumbValidator(
				Breadcrumbs& breadcrumbs,
				const std::string& name) {
			return std::make_unique<MockBreadcrumbValidator<TNotification>>(name, breadcrumbs);
		}
	}

	TEST(TEST_CLASS, CanFilterValidatorsBasedOnNotificationType) {
		// Arrange:
		Breadcrumbs breadcrumbs;
		stateful::DemuxValidatorBuilder builder;

		auto cache = test::CreateEmptyCatapultCache();
		auto cacheView = cache.createView();
		auto context = test::CreateValidatorContext(Height(123), cacheView.toReadOnly());

		builder
			.add(CreateBreadcrumbValidator<model::AccountPublicKeyNotification>(breadcrumbs, "alpha"))
			.add(CreateBreadcrumbValidator<model::AccountAddressNotification>(breadcrumbs, "OMEGA"))
			.add(CreateBreadcrumbValidator(breadcrumbs, "zEtA"));
		auto pValidator = builder.build();

		// Act:
		auto notification = model::AccountPublicKeyNotification(Key());
		auto result = test::ValidateNotification<model::Notification>(*pValidator, notification, context);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);

		Breadcrumbs expectedNames{ "alpha", "OMEGA", "zEtA" };
		EXPECT_EQ(expectedNames, pValidator->names());

		// - alpha matches notification type
		// - OMEGA does not match notification type
		// - zEtA matches all types
		Breadcrumbs expectedSelectedNames{ "alpha", "zEtA" };
		EXPECT_EQ(expectedSelectedNames, breadcrumbs);
	}

	// endregion
}}
