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
#include "catapult/crypto/KeyPair.h"

namespace catapult { namespace harvesting {

	/// Contains all account dependent information required to generate a block.
	class BlockGeneratorAccountDescriptor {
	public:
		/// Creates default descriptor.
		BlockGeneratorAccountDescriptor();

		/// Creates descriptor around \a signingKeyPair and \a vrfKeyPair.
		BlockGeneratorAccountDescriptor(crypto::KeyPair&& signingKeyPair, crypto::KeyPair&& vrfKeyPair);

	public:
		/// Gets the signing key pair.
		const crypto::KeyPair& signingKeyPair() const;

		/// Gets the vrf key pair.
		const crypto::KeyPair& vrfKeyPair() const;

	public:
		/// Returns \c true if this descriptor is equal to \a rhs.
		bool operator==(const BlockGeneratorAccountDescriptor& rhs) const;

		/// Returns \c true if this descriptor is not equal to \a rhs.
		bool operator!=(const BlockGeneratorAccountDescriptor& rhs) const;

	private:
		crypto::KeyPair m_signingKeyPair;
		crypto::KeyPair m_vrfKeyPair;
	};
}}
