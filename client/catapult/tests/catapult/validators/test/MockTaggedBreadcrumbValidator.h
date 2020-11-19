/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
