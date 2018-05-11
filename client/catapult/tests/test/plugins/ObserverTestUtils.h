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

#pragma once
#include "ObserverTestContext.h"
#include "catapult/observers/NotificationObserver.h"

namespace catapult { namespace test {

	// region NotificationObserver

	/// Observes \a notification with \a observer using \a context.
	template<typename TNotification>
	void ObserveNotification(
			const observers::NotificationObserverT<TNotification>& observer,
			const TNotification& notification,
			const observers::ObserverContext& context) {
		observer.notify(notification, context);
	}

	/// Observes \a notification with \a observer using \a context.
	template<typename TNotification, typename TCacheFactory>
	void ObserveNotification(
			const observers::NotificationObserverT<TNotification>& observer,
			const TNotification& notification,
			const ObserverTestContextT<TCacheFactory>& context) {
		ObserveNotification(observer, notification, context.observerContext());
	}

	// endregion

	/// Creates an observer context around \a state at \a height with specified \a mode.
	constexpr observers::ObserverContext CreateObserverContext(
			const observers::ObserverState& state,
			Height height,
			observers::NotifyMode mode) {
		return observers::ObserverContext(state, height, mode);
	}

	/// Creates an observer context around \a cache and \a state at \a height with specified \a mode.
	constexpr observers::ObserverContext CreateObserverContext(
			cache::CatapultCacheDelta& cache,
			state::CatapultState& state,
			Height height,
			observers::NotifyMode mode) {
		return observers::ObserverContext(cache, state, height, mode);
	}

/// Defines common observer tests for an observer with \a NAME.
#define DEFINE_COMMON_OBSERVER_TESTS(NAME, ...) \
	TEST(NAME##ObserverTests, CanCreate##NAME##Observer) { \
		auto pObserver = Create##NAME##Observer(__VA_ARGS__); \
		EXPECT_EQ(#NAME "Observer", pObserver->name()); \
	}
}}
