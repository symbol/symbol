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
#include "BmPrivateKeyTree.h"

namespace catapult { namespace crypto {

	/// Aggregate of multiple two-layer Bellare-Miner private key trees.
	class AggregateBmPrivateKeyTree {
	private:
		using BmPublicKey = decltype(BmTreeSignature::Root.ParentPublicKey);
		using PrivateKeyTreeFactory = supplier<std::unique_ptr<BmPrivateKeyTree>>;

	public:
		/// Creates a tree around \a factory.
		explicit AggregateBmPrivateKeyTree(const PrivateKeyTreeFactory& factory);

	public:
		/// Gets the root public key.
		const BmPublicKey& rootPublicKey() const;

		/// Gets the options.
		const BmOptions& options() const;

		/// Returns \c true if can sign at \a keyIdentifier.
		bool canSign(const BmKeyIdentifier& keyIdentifier);

		/// Creates the signature for \a dataBuffer at \a keyIdentifier.
		BmTreeSignature sign(const BmKeyIdentifier& keyIdentifier, const RawBuffer& dataBuffer);

	private:
		PrivateKeyTreeFactory m_factory;
		std::unique_ptr<BmPrivateKeyTree> m_pTree;
	};
}}
