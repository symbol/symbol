#pragma once
#include "ObserverTypes.h"
#include "catapult/model/NotificationPublisher.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace observers {

	/// A notification observer to entity observer adapter.
	class NotificationObserverAdapter : public EntityObserver {
	private:
		using NotificationObserverPointer = NotificationObserverPointerT<model::Notification>;
		using NotificationPublisherPointer = std::unique_ptr<const model::NotificationPublisher>;

	public:
		/// Creates a new adapter around \a pObserver and \a pPublisher.
		NotificationObserverAdapter(NotificationObserverPointer&& pObserver, NotificationPublisherPointer&& pPublisher);

	public:
		const std::string& name() const override;

		void notify(const model::WeakEntityInfo& entityInfo, const ObserverContext& context) const override;

	private:
		NotificationObserverPointer m_pObserver;
		NotificationPublisherPointer m_pPublisher;
	};
}}
