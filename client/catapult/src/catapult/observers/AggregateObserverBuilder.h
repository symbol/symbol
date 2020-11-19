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
#include "ObserverTypes.h"
#include "catapult/utils/NamedObject.h"
#include <vector>

namespace catapult { namespace observers {

	/// Strongly typed aggregate notification observer builder.
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

			void notify(const TNotification& notification, ObserverContext& context) const override {
				if (NotifyMode::Commit == context.Mode)
					notifyAll(m_observers.cbegin(), m_observers.cend(), notification, context);
				else
					notifyAll(m_observers.crbegin(), m_observers.crend(), notification, context);
			}

		private:
			template<typename TIter>
			void notifyAll(TIter begin, TIter end, const TNotification& notification, ObserverContext& context) const {
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
