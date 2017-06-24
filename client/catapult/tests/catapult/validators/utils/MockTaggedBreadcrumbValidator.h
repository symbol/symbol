#pragma once
#include "catapult/validators/NotificationValidator.h"
#include "catapult/validators/ValidatorTypes.h"
#include "tests/test/core/TaggedNotification.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Creates a (type 1) tagged breadcrumb validator with \a tag and \a breadcrumbs and optional \a result.
	validators::stateful::NotificationValidatorPointerT<test::TaggedNotification> CreateTaggedBreadcrumbValidator(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs,
			validators::ValidationResult result = validators::ValidationResult::Success);

	/// Creates a (type 2) tagged breadcrumb validator with \a tag and \a breadcrumbs and optional \a result.
	validators::stateful::NotificationValidatorPointerT<test::TaggedNotification2> CreateTaggedBreadcrumbValidator2(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs,
			validators::ValidationResult result = validators::ValidationResult::Success);
}}
