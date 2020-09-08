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

#include "AggregateBmPrivateKeyTree.h"
#include "catapult/exceptions.h"

namespace catapult { namespace crypto {

	AggregateBmPrivateKeyTree::AggregateBmPrivateKeyTree(BmPrivateKeyTree&& tree) : m_tree(std::move(tree))
	{}

	AggregateBmPrivateKeyTree AggregateBmPrivateKeyTree::FromStream(io::SeekableStream& storage) {
		return AggregateBmPrivateKeyTree(BmPrivateKeyTree::FromStream(storage));
	}

	AggregateBmPrivateKeyTree AggregateBmPrivateKeyTree::Create(
			BmKeyPair&& keyPair,
			io::SeekableStream& storage,
			const BmOptions& options) {
		return AggregateBmPrivateKeyTree(BmPrivateKeyTree::Create(std::move(keyPair), storage, options));
	}

	const AggregateBmPrivateKeyTree::BmPublicKey& AggregateBmPrivateKeyTree::rootPublicKey() const {
		return m_tree.rootPublicKey();
	}

	const BmOptions& AggregateBmPrivateKeyTree::options() const {
		return m_tree.options();
	}

	bool AggregateBmPrivateKeyTree::canSign(const BmKeyIdentifier& keyIdentifier) const {
		return m_tree.canSign(keyIdentifier);
	}

	namespace {
		BmKeyIdentifier Decrease(const BmKeyIdentifier& keyIdentifier) {
			return { keyIdentifier.BatchId, 0 == keyIdentifier.KeyId ? BmKeyIdentifier::Invalid_Id: keyIdentifier.KeyId - 1 };
		}
	}

	BmTreeSignature AggregateBmPrivateKeyTree::sign(const BmKeyIdentifier& keyIdentifier, const RawBuffer& dataBuffer) {
		auto signature = m_tree.sign(keyIdentifier, dataBuffer);

		if (keyIdentifier != m_tree.options().StartKeyIdentifier)
			m_tree.wipe(Decrease(keyIdentifier));

		return signature;
	}
}}
