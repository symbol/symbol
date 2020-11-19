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

#include "BlockGeneratorAccountDescriptor.h"

namespace catapult { namespace harvesting {

	namespace {
		crypto::KeyPair CreateZeroKeyPair() {
			return crypto::KeyPair::FromPrivate(crypto::PrivateKey());
		}
	}

	BlockGeneratorAccountDescriptor::BlockGeneratorAccountDescriptor()
			: BlockGeneratorAccountDescriptor(CreateZeroKeyPair(), CreateZeroKeyPair())
	{}

	BlockGeneratorAccountDescriptor::BlockGeneratorAccountDescriptor(crypto::KeyPair&& signingKeyPair, crypto::KeyPair&& vrfKeyPair)
			: m_signingKeyPair(std::move(signingKeyPair))
			, m_vrfKeyPair(std::move(vrfKeyPair))
	{}

	const crypto::KeyPair& BlockGeneratorAccountDescriptor::signingKeyPair() const {
		return m_signingKeyPair;
	}

	const crypto::KeyPair& BlockGeneratorAccountDescriptor::vrfKeyPair() const {
		return m_vrfKeyPair;
	}

	bool BlockGeneratorAccountDescriptor::operator==(const BlockGeneratorAccountDescriptor& rhs) const {
		return m_signingKeyPair.publicKey() == rhs.m_signingKeyPair.publicKey()
				&& m_vrfKeyPair.publicKey() == rhs.m_vrfKeyPair.publicKey();
	}

	bool BlockGeneratorAccountDescriptor::operator!=(const BlockGeneratorAccountDescriptor& rhs) const {
		return !(*this == rhs);
	}
}}
