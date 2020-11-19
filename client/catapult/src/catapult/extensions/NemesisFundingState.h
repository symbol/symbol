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
#include "catapult/state/AccountBalances.h"

namespace catapult { namespace extensions {

	/// Possible nemesis funding types.
	enum class NemesisFundingType {
		/// Unknown (e.g. funding type has not yet been determined).
		Unknown,

		/// Explicitly funded (e.g. mosaic supply transaction).
		Explicit,

		/// Implicitly funded (e.g. balance transfers).
		Implicit
	};

	/// State used by the nemesis funding observer.
	struct NemesisFundingState {
	public:
		/// Creates a default nemesis funding state.
		NemesisFundingState() : FundingType(NemesisFundingType::Unknown)
		{}

	public:
		/// Total sums of mosaics credited in nemesis block.
		/// \note Only sum of harvesting mosaic is used. Rest are accumulated for diagnostics.
		state::AccountBalances TotalFundedMosaics;

		/// Nemesis block funding type.
		NemesisFundingType FundingType;
	};
}}
