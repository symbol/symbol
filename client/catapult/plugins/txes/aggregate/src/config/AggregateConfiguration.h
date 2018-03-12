#pragma once
#include <stdint.h>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Aggregate plugin configuration settings.
	struct AggregateConfiguration {
	public:
		/// The maximum number of transactions per aggregate.
		uint32_t MaxTransactionsPerAggregate;

		/// The maximum number of cosignatures per aggregate.
		uint8_t MaxCosignaturesPerAggregate;

		/// \c true if cosignatures must exactly match component signers.
		/// \c false if cosignatures should be validated externally.
		bool EnableStrictCosignatureCheck;

		/// \c true if bonded aggregates should be allowed.
		/// \c false if bonded aggregates should be rejected.
		bool EnableBondedAggregateSupport;

	private:
		AggregateConfiguration() = default;

	public:
		/// Creates an uninitialized aggregate configuration.
		static AggregateConfiguration Uninitialized();

		/// Loads an aggregate configuration from \a bag.
		static AggregateConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
