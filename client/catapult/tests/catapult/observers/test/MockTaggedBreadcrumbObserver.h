#pragma once
#include "catapult/observers/NotificationObserver.h"
#include "catapult/observers/ObserverTypes.h"
#include "tests/test/core/TaggedNotification.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Creates a (type 1) tagged breadcrumb observer with \a tag and \a breadcrumbs.
	observers::NotificationObserverPointerT<test::TaggedNotification> CreateTaggedBreadcrumbObserver(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs);

	/// Creates a (type 2) tagged breadcrumb observer with \a tag and \a breadcrumbs.
	observers::NotificationObserverPointerT<test::TaggedNotification2> CreateTaggedBreadcrumbObserver2(
			uint8_t tag,
			std::vector<uint16_t>& breadcrumbs);
}}
