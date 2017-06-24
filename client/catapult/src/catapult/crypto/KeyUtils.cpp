#include "KeyUtils.h"
#include "PrivateKey.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace crypto {

	utils::ContainerHexFormatter<Key::const_iterator> FormatKey(const Key& key) {
		return utils::HexFormat(key);
	}

	utils::ContainerHexFormatter<Key::const_iterator> FormatKey(const PrivateKey& key) {
		return utils::HexFormat(key.cbegin(), key.cend());
	}

	Key ParseKey(const std::string& keyString) {
		Key key;
		utils::ParseHexStringIntoContainer(keyString.c_str(), keyString.size(), key);
		return key;
	}
}}
