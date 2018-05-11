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

#include "catapult/validators/AggregateEntityValidator.h"
#include "catapult/validators/ValidatorTypes.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateEntityValidatorTests

	namespace {
		using Breadcrumbs = std::vector<std::string>;

		class MockBreadcrumbValidator : public stateful::EntityValidator {
		public:
			MockBreadcrumbValidator(const std::string& name, Breadcrumbs& breadcrumbs)
					: m_name(name)
					, m_breadcrumbs(breadcrumbs)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(const model::WeakEntityInfo&, const ValidatorContext& context) const override {
				m_breadcrumbs.push_back(m_name + std::to_string(context.Height.unwrap()));
				return ValidationResult::Success;
			}

		private:
			std::string m_name;
			Breadcrumbs& m_breadcrumbs;
		};

		std::unique_ptr<const stateful::EntityValidator> CreateBreadcrumbValidator(Breadcrumbs& breadcrumbs, const std::string& name) {
			return std::make_unique<MockBreadcrumbValidator>(name, breadcrumbs);
		}

		void Validate(const stateful::AggregateEntityValidator& validator, const model::VerifiableEntity& entity) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto context = test::CreateValidatorContext(Height(123), cacheView.toReadOnly());
			validator.curry(std::cref(context)).dispatch([&entity](const auto&, const auto& validationFunctions) {
				Hash256 hash;
				auto entityInfo = model::WeakEntityInfoT<model::VerifiableEntity>(entity, hash);

				// Act: just invoke every validation function once
				auto i = 0u;
				for (const auto& validationFunction : validationFunctions) {
					auto result = validationFunction(entityInfo);

					// Sanity: all functions should succeed
					EXPECT_EQ(ValidationResult::Success, result) << "validation function at " << i;
					++i;
				}
			}, {});
		}

		void Validate(const stateful::AggregateEntityValidator& validator) {
			// Act:
			Validate(validator, model::VerifiableEntity());
		}

		using AggregateEntityValidatorBuilder = ValidatorVectorT<const ValidatorContext&>;

		auto CreateAggregateValidator(AggregateEntityValidatorBuilder&& builder) {
			return std::make_unique<stateful::AggregateEntityValidator>(std::move(builder));
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyAggregateValidator) {
		// Arrange:
		AggregateEntityValidatorBuilder builder;

		// Act:
		auto pValidator = CreateAggregateValidator(std::move(builder));

		// Assert:
		EXPECT_TRUE(pValidator->names().empty());
	}

	TEST(TEST_CLASS, CanCreateWithSingleValidator) {
		// Arrange:
		Breadcrumbs breadcrumbs;
		AggregateEntityValidatorBuilder builder;

		// Act:
		builder.push_back(CreateBreadcrumbValidator(breadcrumbs, "alpha"));
		auto pValidator = CreateAggregateValidator(std::move(builder));
		Validate(*pValidator);

		// Assert: notice that breadcrumbs include the height from the context
		EXPECT_EQ(Breadcrumbs{ "alpha" }, pValidator->names());
		EXPECT_EQ(Breadcrumbs{ "alpha123" }, breadcrumbs);
	}

	TEST(TEST_CLASS, CanCreateWithMultipleValidators) {
		// Arrange:
		Breadcrumbs breadcrumbs;
		AggregateEntityValidatorBuilder builder;

		// Act:
		builder.push_back(CreateBreadcrumbValidator(breadcrumbs, "alpha"));
		builder.push_back(CreateBreadcrumbValidator(breadcrumbs, "OMEGA"));
		builder.push_back(CreateBreadcrumbValidator(breadcrumbs, "zEtA"));
		auto pValidator = CreateAggregateValidator(std::move(builder));
		Validate(*pValidator);

		// Assert: notice that breadcrumbs include the height from the context
		EXPECT_EQ(Breadcrumbs({ "alpha", "OMEGA", "zEtA" }), pValidator->names());
		EXPECT_EQ(Breadcrumbs({ "alpha123", "OMEGA123", "zEtA123" }), breadcrumbs);
	}
}}
