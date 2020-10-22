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
#include "BmOptions.h"
#include "BmTreeSignature.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/io/SeekableStream.h"
#include <memory>

namespace catapult { namespace crypto {

	/// Two-layer Bellare-Miner private key tree.
	class BmPrivateKeyTree {
	private:
		using BmKeyPair = VotingKeyPair;

	private:
		BmPrivateKeyTree(io::SeekableStream& storage, const BmOptions& options);

	public:
		/// Move constructor.
		BmPrivateKeyTree(BmPrivateKeyTree&& tree);

	public:
		/// Destroys the tree.
		~BmPrivateKeyTree();

	public:
		/// Creates a tree around \a storage.
		static BmPrivateKeyTree FromStream(io::SeekableStream& storage);

		/// Creates a tree around \a keyPair, \a storage and \a options.
		static BmPrivateKeyTree Create(BmKeyPair&& keyPair, io::SeekableStream& storage, const BmOptions& options);

	public:
		/// Gets the root public key.
		const decltype(BmTreeSignature::Root.ParentPublicKey)& rootPublicKey() const;

		/// Gets the options.
		const BmOptions& options() const;

		/// Returns \c true if can sign at \a keyIdentifier.
		bool canSign(const BmKeyIdentifier& keyIdentifier) const;

		/// Creates the signature for \a dataBuffer at \a keyIdentifier.
		BmTreeSignature sign(const BmKeyIdentifier& keyIdentifier, const RawBuffer& dataBuffer);

		/// Wipes all keys up to and including \a keyIdentifier.
		void wipe(const BmKeyIdentifier& keyIdentifier);

	private:
		class Level;

	private:
		bool check(const BmKeyIdentifier& keyIdentifier, const BmKeyIdentifier& referenceKeyIdentifier) const;
		size_t levelOffset(size_t depth) const;

		void createLevel(size_t depth, BmKeyPair&& keyPair, uint64_t startIdentifier, uint64_t endIdentifier);
		void wipe(size_t depth, uint64_t identifier);

	private:
		io::SeekableStream& m_storage;
		BmOptions m_options;

		std::array<std::unique_ptr<Level>, 2> m_levels;
		BmKeyIdentifier m_lastKeyIdentifier;
		BmKeyIdentifier m_lastWipeKeyIdentifier;
	};

	/// Verifies \a signature of \a buffer at \a keyIdentifier.
	bool Verify(const BmTreeSignature& signature, const BmKeyIdentifier& keyIdentifier, const RawBuffer& buffer);

	/// Verifies \a signature of \a buffer at \a keyIdentifier.
	bool Verify(const BmTreeSignatureV1& signature, const BmKeyIdentifier& keyIdentifier, const RawBuffer& buffer);
}}
