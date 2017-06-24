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
