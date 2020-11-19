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
#include "catapult/types.h"
#include <boost/program_options.hpp>

namespace catapult { namespace tools {

	/// Options builder.
	using OptionsBuilder = boost::program_options::options_description_easy_init;

	/// Definition of positional arguments.
	using OptionsPositional = boost::program_options::positional_options_description;

	/// Provides access to parsed options.
	using Options = boost::program_options::variables_map;

	/// Helper wrapper to simplify defining values with no specified storage.
	template<typename TValue>
	auto OptionsValue() {
		return boost::program_options::value<TValue>();
	}

	/// Helper wrapper to simplify defining values where \a value is used as storage.
	template<typename TValue>
	auto OptionsValue(TValue& value) {
		return boost::program_options::value<TValue>(&value);
	}

	/// Helper wrapper to simplify defining toggle values.
	inline auto OptionsSwitch() {
		return boost::program_options::bool_switch();
	}
}}
