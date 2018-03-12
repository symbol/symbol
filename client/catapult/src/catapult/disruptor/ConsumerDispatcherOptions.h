#pragma once
#include <stddef.h>

namespace catapult { namespace disruptor {

	/// Consumer dispatcher options.
	struct ConsumerDispatcherOptions {
	public:
		/// Creates options around \a dispatcherName and \a disruptorSize.
		constexpr ConsumerDispatcherOptions(const char* dispatcherName, size_t disruptorSize)
				: DispatcherName(dispatcherName)
				, DisruptorSize(disruptorSize)
				, ElementTraceInterval(1)
				, ShouldThrowIfFull(true)
		{}

	public:
		/// The name of the dispatcher.
		const char* DispatcherName;

		/// The disruptor size.
		size_t DisruptorSize;

		/// The multiple of elements at which an element should be traced through queue and completion.
		size_t ElementTraceInterval;

		/// \c true if the dispatcher should throw if full, \c false if it should return an error.
		bool ShouldThrowIfFull;
	};
}}
