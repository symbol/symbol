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
#include "OtsTypes.h"
#include "SeekableOutputStream.h"
#include "catapult/crypto/KeyPair.h"
#include <array>
#include <memory>

namespace catapult { namespace crypto {

	using OtsKeyPairType = KeyPair;

	/// One time signature tree.
	class OtsTree {
	private:
		OtsTree(SeekableOutputStream& storage, const OtsOptions& options);
		OtsTree(OtsTree&& tree);

	public:
		/// Destroys the tree.
		~OtsTree();

	public:
		/// Creates a tree around \a storage, loading stored data from \a input.
		static OtsTree FromStream(io::InputStream& input, SeekableOutputStream& storage);

		/// Creates a tree around \a keyPair, \a storage, \a startPoint, \a endPoint and \a options.
		static OtsTree Create(
				OtsKeyPairType&& keyPair,
				SeekableOutputStream& storage,
				FinalizationPoint startPoint,
				FinalizationPoint endPoint,
				const OtsOptions& options);

	public:
		/// Gets the root public key.
		const OtsPublicKey& rootPublicKey() const;

		/// Returns \c true if can sign at \a stepIdentifier.
		bool canSign(const StepIdentifier& stepIdentifier) const;

		/// Creates the signature for \a dataBuffer at \a stepIdentifier.
		OtsTreeSignature sign(const StepIdentifier& stepIdentifier, const RawBuffer& dataBuffer);

	private:
		class OtsLevel;

	private:
		size_t levelOffset(size_t depth) const;
		OtsKeyPairType detachKeyPair(size_t depth, uint64_t identifier);
		void createLevel(size_t depth, KeyPair&& keyPair, uint64_t startIdentifier, uint64_t endIdentifier);

	private:
		SeekableOutputStream& m_storage;
		OtsOptions m_options;

		std::array<std::unique_ptr<OtsLevel>, 3> m_levels;
		StepIdentifier m_lastStep;
	};

	/// Verifies \a signature of \a buffer at \a stepIdentifier.
	bool Verify(const OtsTreeSignature& signature, const StepIdentifier& stepIdentifier, const RawBuffer& buffer);
}}
