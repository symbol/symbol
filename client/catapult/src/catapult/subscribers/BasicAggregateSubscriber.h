#pragma once
#include <memory>
#include <vector>

namespace catapult { namespace subscribers {

	/// Basic aggregate subscriber.
	template<typename TSubscriber>
	class BasicAggregateSubscriber {
	public:
		/// Creates an aggregate subscriber around \a subscribers.
		explicit BasicAggregateSubscriber(std::vector<std::unique_ptr<TSubscriber>>&& subscribers)
				: m_subscribers(std::move(subscribers))
		{}

	protected:
		template<typename TAction>
		void forEach(TAction action) const {
			for (auto& pSubscriber : m_subscribers)
				action(*pSubscriber);
		}

	private:
		std::vector<std::unique_ptr<TSubscriber>> m_subscribers;
	};
}}
