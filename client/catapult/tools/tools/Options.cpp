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

#include "Options.h"
#include "ToolKeys.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace tools {

	Key GetKeyFromOptions(const Options& options) {
		auto key = crypto::ParseKey(options[Server_Public_Key_Option_Name].as<std::string>().c_str());
		auto formattedKey = crypto::FormatKey(key);
		if (options.count(Server_Public_Key_Option_Name))
			CATAPULT_LOG(info) << "using server public key: " << formattedKey;
		else
			CATAPULT_LOG(warning) << "using default server public key: " << formattedKey;
		return key;
	}

	void AddDefaultServerKeyOption(OptionsBuilder& optionsBuilder, std::string& key) {
		std::stringstream defaultServerKey;
		defaultServerKey << crypto::FormatKey(LoadServerKeyPair().publicKey());

		// "key,k"
		std::string optionName = Server_Public_Key_Option_Name;
		optionName.push_back(',');
		optionName.push_back(Server_Public_Key_Option_Name[0]);
		optionsBuilder(optionName.c_str(),
				OptionsValue<std::string>(key)->default_value(defaultServerKey.str()),
				"set server public key");
	}
}}
