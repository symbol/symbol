#include "NetworkInfo.h"
#include "catapult/utils/ConfigurationValueParsers.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace model {

#define DEFINE_ENUM NetworkIdentifier
#define EXPLICIT_VALUE_ENUM
#define ENUM_LIST NETWORK_IDENTIFIER_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_VALUE_ENUM
#undef DEFINE_ENUM

	namespace {
		const std::array<std::pair<const char*, NetworkIdentifier>, 4> String_To_Network_Identifier_Pairs{{
			{ "mijin", NetworkIdentifier::Mijin },
			{ "mijin-test", NetworkIdentifier::Mijin_Test },
			{ "public", NetworkIdentifier::Public },
			{ "public-test", NetworkIdentifier::Public_Test }
		}};
	}

	bool TryParseValue(const std::string& networkName, NetworkIdentifier& networkIdentifier) {
		if (utils::TryParseEnumValue(String_To_Network_Identifier_Pairs, networkName, networkIdentifier))
			return true;

		uint8_t rawNetworkIdentifier;
		if (!utils::TryParseValue(networkName, rawNetworkIdentifier))
			return false;

		networkIdentifier = static_cast<NetworkIdentifier>(rawNetworkIdentifier);
		return true;
	}
}}
