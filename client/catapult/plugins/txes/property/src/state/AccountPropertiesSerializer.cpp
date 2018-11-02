/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "AccountPropertiesSerializer.h"
#include "src/state/AccountProperties.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace state {

	void AccountPropertiesSerializer::Save(const AccountProperties& accountProperties, io::OutputStream& output) {
		io::Write(output, accountProperties.address());

		io::Write64(output, accountProperties.size());
		for (const auto& pair : accountProperties) {
			const auto& accountProperty = pair.second;
			io::Write8(output, utils::to_underlying_type(accountProperty.descriptor().raw()));
			io::Write64(output, accountProperty.values().size());
			for (const auto& value : accountProperty.values())
				output.write(value);
		}
	}

	AccountProperties AccountPropertiesSerializer::Load(io::InputStream& input) {
		Address address;
		input.read(address);

		auto accountProperties = AccountProperties(address);

		auto numProperties = io::Read64(input);
		for (auto i = 0u; i < numProperties; ++i) {
			auto propertyDescriptor = state::PropertyDescriptor(static_cast<model::PropertyType>(io::Read8(input)));
			auto& accountProperty = accountProperties.property(propertyDescriptor.propertyType());

			AccountProperty::RawPropertyValue value(accountProperty.propertyValueSize());
			auto numValues = io::Read64(input);
			for (auto j = 0u; j < numValues; ++j) {
				input.read(value);
				model::RawPropertyModification modification{ model::PropertyModificationType::Add, value };
				if (OperationType::Allow == propertyDescriptor.operationType())
					accountProperty.allow(modification);
				else
					accountProperty.block(modification);
			}
		}

		return accountProperties;
	}
}}
