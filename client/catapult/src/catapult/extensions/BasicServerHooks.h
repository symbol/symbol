#pragma once
#include "catapult/exceptions.h"
#include <vector>

namespace catapult { namespace extensions {

	/// Sets \a dest to \a source if and only if \a dest is unset.
	template<typename TFunc>
	void SetOnce(TFunc& dest, const TFunc& source) {
		if (dest)
			CATAPULT_THROW_INVALID_ARGUMENT("server hook cannot be set more than once");

		dest = source;
	}

	/// Returns \a func if and only if it is set.
	template<typename TFunc>
	const TFunc& Require(const TFunc& func) {
		if (!func)
			CATAPULT_THROW_INVALID_ARGUMENT("server hook has not been set");

		return func;
	}

	/// Aggregates multiple \a consumers into a single consumer.
	template<typename TConsumer>
	static TConsumer AggregateConsumers(const std::vector<TConsumer>& consumers) {
		return [consumers](const auto& data) {
			for (const auto& consumer : consumers)
				consumer(data);
		};
	}
}}
