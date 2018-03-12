#include "KeyUtils.h"
#include "PrivateKey.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace crypto {

	utils::ContainerHexFormatter<Key::const_iterator> FormatKey(const Key& key) {
		return utils::HexFormat(key);
	}

	utils::ContainerHexFormatter<Key::const_iterator> FormatKey(const PrivateKey& key) {
		return utils::HexFormat(key.begin(), key.end());
	}

	Key ParseKey(const std::string& keyString) {
		Key key;
		utils::ParseHexStringIntoContainer(keyString.c_str(), keyString.size(), key);
		return key;
	}

	bool IsValidKeyString(const std::string& str) {
		Key key;
		return utils::TryParseHexStringIntoContainer(str.data(), str.size(), key);
	}
}}
