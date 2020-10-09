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

#include "NetworkIdentifier.h"
#include "catapult/utils/ConfigurationValueParsers.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace model {

	// region NetworkIdentifier

#define DEFINE_ENUM NetworkIdentifier
#define EXPLICIT_VALUE_ENUM
#define ENUM_LIST NETWORK_IDENTIFIER_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_VALUE_ENUM
#undef DEFINE_ENUM

	namespace {
		const std::array<std::pair<const char*, NetworkIdentifier>, 6> String_To_Network_Identifier_Pairs{{
			{ "mijin", NetworkIdentifier::Mijin },
			{ "mijin-test", NetworkIdentifier::Mijin_Test },
			{ "private", NetworkIdentifier::Private },
			{ "private-test", NetworkIdentifier::Private_Test },
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

	// endregion

	// region UniqueNetworkFingerprint

	UniqueNetworkFingerprint::UniqueNetworkFingerprint() : UniqueNetworkFingerprint(NetworkIdentifier::Zero)
	{}

	UniqueNetworkFingerprint::UniqueNetworkFingerprint(NetworkIdentifier identifier)
			: UniqueNetworkFingerprint(identifier, catapult::GenerationHashSeed())
	{}

	UniqueNetworkFingerprint::UniqueNetworkFingerprint(
			NetworkIdentifier identifier,
			const catapult::GenerationHashSeed& generationHashSeed)
			: Identifier(identifier)
			, GenerationHashSeed(generationHashSeed)
	{}

	bool UniqueNetworkFingerprint::operator==(const UniqueNetworkFingerprint& rhs) const {
		return Identifier == rhs.Identifier && GenerationHashSeed == rhs.GenerationHashSeed;
	}

	bool UniqueNetworkFingerprint::operator!=(const UniqueNetworkFingerprint& rhs) const {
		return !(*this == rhs);
	}

	std::ostream& operator<<(std::ostream& out, const UniqueNetworkFingerprint& fingerprint) {
		out << fingerprint.Identifier << "::" << fingerprint.GenerationHashSeed;
		return out;
	}

	// endregion
}}
