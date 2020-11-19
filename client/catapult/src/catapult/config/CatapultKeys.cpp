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

#include "CatapultKeys.h"
#include "catapult/crypto/OpensslKeyUtils.h"
#include <filesystem>

namespace catapult { namespace config {

	CatapultKeys::CatapultKeys() : m_nodeKeyPair(crypto::KeyPair::FromPrivate(crypto::PrivateKey()))
	{}

	CatapultKeys::CatapultKeys(const std::string& directory)
			: m_caPublicKey(crypto::ReadPublicKeyFromPublicKeyPemFile(GetCaPublicKeyPemFilename(directory)))
			, m_nodeKeyPair(crypto::ReadKeyPairFromPrivateKeyPemFile(GetNodePrivateKeyPemFilename(directory)))
	{}

	CatapultKeys::CatapultKeys(Key&& caPublicKey, crypto::KeyPair&& nodeKeyPair)
			: m_caPublicKey(std::move(caPublicKey))
			, m_nodeKeyPair(std::move(nodeKeyPair))
	{}

	const Key& CatapultKeys::caPublicKey() const {
		return m_caPublicKey;
	}

	const crypto::KeyPair& CatapultKeys::nodeKeyPair() const {
		return m_nodeKeyPair;
	}

	namespace {
		std::string GetPemFilename(const std::string& directory, const std::string& name) {
			return (std::filesystem::path(directory) / (name + ".pem")).generic_string();
		}
	}

	std::string GetCaPublicKeyPemFilename(const std::string& directory) {
		return GetPemFilename(directory, "ca.pubkey");
	}

	std::string GetNodePrivateKeyPemFilename(const std::string& directory) {
		return GetPemFilename(directory, "node.key");
	}
}}
