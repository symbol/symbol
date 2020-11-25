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

#include "AccountRestrictionsSerializer.h"
#include "src/state/AccountRestrictions.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace state {

	void AccountRestrictionsSerializer::Save(const AccountRestrictions& restrictions, io::OutputStream& output) {
		output.write(restrictions.address());

		auto shouldWriteEmpty = 1 >= restrictions.version();

		auto numRestrictions = static_cast<uint64_t>(std::count_if(restrictions.begin(), restrictions.end(), [shouldWriteEmpty](
				const auto& pair) {
			return shouldWriteEmpty || !pair.second.values().empty();
		}));

		io::Write64(output, numRestrictions);
		for (const auto& pair : restrictions) {
			const auto& restriction = pair.second;
			if (!shouldWriteEmpty && restriction.values().empty())
				continue;

			io::Write16(output, utils::to_underlying_type(restriction.descriptor().raw()));
			io::Write64(output, restriction.values().size());
			for (const auto& value : restriction.values())
				output.write(value);
		}
	}

	AccountRestrictions AccountRestrictionsSerializer::Load(io::InputStream& input) {
		Address address;
		input.read(address);

		AccountRestrictions restrictions(address);

		auto numRestrictions = io::Read64(input);
		for (auto i = 0u; i < numRestrictions; ++i) {
			auto restrictionFlags = static_cast<model::AccountRestrictionFlags>(io::Read16(input));
			auto restrictionDescriptor = state::AccountRestrictionDescriptor(restrictionFlags);
			auto& restriction = restrictions.restriction(restrictionDescriptor.directionalRestrictionFlags());

			AccountRestriction::RawValue value(restriction.valueSize());
			auto numValues = io::Read64(input);
			for (auto j = 0u; j < numValues; ++j) {
				input.read(value);
				model::AccountRestrictionModification modification{ model::AccountRestrictionModificationAction::Add, value };
				if (AccountRestrictionOperationType::Allow == restrictionDescriptor.operationType())
					restriction.allow(modification);
				else
					restriction.block(modification);
			}
		}

		return restrictions;
	}
}}
