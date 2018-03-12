#pragma once
#include "catapult/model/NotificationPublisher.h"

namespace catapult { namespace mocks {

	/// A mock notification publisher that counts the number of publish calls.
	class MockNotificationPublisher : public model::NotificationPublisher {
	public:
		/// Creates a mock notification publisher.
		MockNotificationPublisher() : m_numPublishCalls(0)
		{}

	public:
		/// Gets the number of publish calls.
		size_t numPublishCalls() const {
			return m_numPublishCalls;
		}

	public:
		void publish(const model::WeakEntityInfo&, model::NotificationSubscriber&) const override {
			++m_numPublishCalls;
		}

	private:
		mutable size_t m_numPublishCalls;
	};
}}
