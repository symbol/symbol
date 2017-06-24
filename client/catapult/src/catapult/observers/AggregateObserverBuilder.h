#pragma once
#include "ObserverTypes.h"
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace observers {

	/// A strongly typed aggregate notification observer builder.
	template<typename TNotification>
	class AggregateObserverBuilder {
	private:
		using NotificationObserverPointer = NotificationObserverPointerT<TNotification>;
		using NotificationObserverPointerVector = std::vector<NotificationObserverPointer>;

	public:
		/// Adds \a pObserver to the builder and allows chaining.
		AggregateObserverBuilder& add(NotificationObserverPointer&& pObserver) {
			m_observers.push_back(std::move(pObserver));
			return *this;
		}

		/// Builds a strongly typed notification observer.
		AggregateNotificationObserverPointerT<TNotification> build() {
			return std::make_unique<DefaultAggregateNotificationObserver>(std::move(m_observers));
		}

	private:
		class DefaultAggregateNotificationObserver : public AggregateNotificationObserverT<TNotification> {
		public:
			explicit DefaultAggregateNotificationObserver(NotificationObserverPointerVector&& observers)
					: m_observers(std::move(observers))
					, m_name(utils::ReduceNames(utils::ExtractNames(m_observers)))
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			std::vector<std::string> names() const override {
				return utils::ExtractNames(m_observers);
			}

			void notify(const TNotification& notification, const ObserverContext& context) const override {
				if (NotifyMode::Commit == context.Mode)
					notifyAll(m_observers.cbegin(), m_observers.cend(), notification, context);
				else
					notifyAll(m_observers.crbegin(), m_observers.crend(), notification, context);
			}

		private:
			template<typename TIter>
			void notifyAll(TIter begin, TIter end, const TNotification& notification, const ObserverContext& context) const {
				for (auto iter = begin; end != iter; ++iter)
					(*iter)->notify(notification, context);
			}

		private:
			NotificationObserverPointerVector m_observers;
			std::string m_name;
		};

	private:
		NotificationObserverPointerVector m_observers;
	};
}}
