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

#pragma once
#include "catapult/crypto/KeyPair.h"

namespace catapult { namespace config {

	/// Container of keys used by catapult.
	class CatapultKeys {
	public:
		/// Creates empty container.
		CatapultKeys();

		/// Creates a new container and loads keys from \a directory.
		explicit CatapultKeys(const std::string& directory);

		/// Creates a new container around \a caPublicKey and \a nodeKeyPair.
		CatapultKeys(Key&& caPublicKey, crypto::KeyPair&& nodeKeyPair);

	public:
		/// Gets the CA public key.
		const Key& caPublicKey() const;

		/// Gets the node key pair.
		const crypto::KeyPair& nodeKeyPair() const;

	private:
		Key m_caPublicKey;
		crypto::KeyPair m_nodeKeyPair;
	};

	/// Gets the name of the CA public key pem file in \a directory.
	std::string GetCaPublicKeyPemFilename(const std::string& directory);

	/// Gets the name of the node private key pem file in \a directory.
	std::string GetNodePrivateKeyPemFilename(const std::string& directory);
}}
