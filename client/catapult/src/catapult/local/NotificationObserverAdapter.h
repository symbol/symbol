#pragma once
#include "catapult/model/NotificationPublisher.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace model { class TransactionRegistry; } }

namespace catapult { namespace local {

	/// A notification observer to entity observer adapter.
	class NotificationObserverAdapter : public observers::EntityObserver {
	private:
		using NotificationObserverPointer = observers::NotificationObserverPointerT<model::Notification>;

	public:
		/// Creates a new adapter around \a pObserver given \a transactionRegistry.
		explicit NotificationObserverAdapter(
				const model::TransactionRegistry& transactionRegistry,
				NotificationObserverPointer&& pObserver);

	public:
		const std::string& name() const override;

		void notify(const model::WeakEntityInfo& entityInfo, const observers::ObserverContext& context) const override;

	private:
		NotificationObserverPointer m_pObserver;
		std::unique_ptr<model::NotificationPublisher> m_pPub;
	};
}}
