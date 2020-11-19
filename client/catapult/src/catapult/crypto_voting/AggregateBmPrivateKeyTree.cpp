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

#include "AggregateBmPrivateKeyTree.h"
#include "catapult/exceptions.h"

namespace catapult { namespace crypto {

	AggregateBmPrivateKeyTree::AggregateBmPrivateKeyTree(const PrivateKeyTreeFactory& factory)
			: m_factory(factory)
			, m_pTree(m_factory())
	{}

	const AggregateBmPrivateKeyTree::BmPublicKey& AggregateBmPrivateKeyTree::rootPublicKey() const {
		return m_pTree->rootPublicKey();
	}

	const BmOptions& AggregateBmPrivateKeyTree::options() const {
		return m_pTree->options();
	}

	bool AggregateBmPrivateKeyTree::canSign(const BmKeyIdentifier& keyIdentifier) {
		while (m_pTree && m_pTree->options().EndKeyIdentifier < keyIdentifier) {
			auto oldEndKeyIdentifier = m_pTree->options().EndKeyIdentifier;
			m_pTree->wipe(oldEndKeyIdentifier);
			m_pTree = m_factory();
			if (!m_pTree)
				break;

			auto newStartKeyIdentifier = m_pTree->options().StartKeyIdentifier;
			if (oldEndKeyIdentifier >= newStartKeyIdentifier) {
				std::ostringstream out;
				out
						<< "PrivateKeyTreeFactory returned overlapping tree, previous end " << oldEndKeyIdentifier
						<< ", new start " << newStartKeyIdentifier;
				CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
			}
		}

		return m_pTree && m_pTree->canSign(keyIdentifier);
	}

	namespace {
		BmKeyIdentifier Decrease(const BmKeyIdentifier& keyIdentifier) {
			return { keyIdentifier.KeyId - 1 };
		}
	}

	BmTreeSignature AggregateBmPrivateKeyTree::sign(const BmKeyIdentifier& keyIdentifier, const RawBuffer& dataBuffer) {
		if (!canSign(keyIdentifier))
			CATAPULT_THROW_INVALID_ARGUMENT_1("sign called with invalid key identifier", keyIdentifier);

		auto signature = m_pTree->sign(keyIdentifier, dataBuffer);

		if (keyIdentifier != m_pTree->options().StartKeyIdentifier)
			m_pTree->wipe(Decrease(keyIdentifier));

		return signature;
	}
}}
