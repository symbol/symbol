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
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace config {

	/// Aggregate plugin configuration settings.
	struct AggregateConfiguration {
	public:
		/// Maximum number of transactions per aggregate.
		uint32_t MaxTransactionsPerAggregate;

		/// Maximum number of cosignatures per aggregate.
		uint8_t MaxCosignaturesPerAggregate;

		/// \c true if cosignatures must exactly match component signers.
		/// \c false if cosignatures should be validated externally.
		bool EnableStrictCosignatureCheck;

		/// \c true if bonded aggregates should be allowed.
		/// \c false if bonded aggregates should be rejected.
		bool EnableBondedAggregateSupport;

		/// Maximum lifetime a bonded transaction can have before it expires.
		utils::TimeSpan MaxBondedTransactionLifetime;

	private:
		AggregateConfiguration() = default;

	public:
		/// Creates an uninitialized aggregate configuration.
		static AggregateConfiguration Uninitialized();

		/// Loads an aggregate configuration from \a bag.
		static AggregateConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}
