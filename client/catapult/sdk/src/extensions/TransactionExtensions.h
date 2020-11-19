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
#include "catapult/model/Transaction.h"

namespace catapult { namespace extensions {

	/// Extensions for working with transactions.
	class TransactionExtensions {
	public:
		/// Creates extensions for transactions for the network with the specified generation hash seed (\a generationHashSeed).
		explicit TransactionExtensions(const GenerationHashSeed& generationHashSeed);

	public:
		/// Hashes the \a transaction.
		Hash256 hash(const model::Transaction& transaction) const;

		/// Signs the \a transaction using \a signer private key.
		void sign(const crypto::KeyPair& signer, model::Transaction& transaction) const;

		/// Verifies signature of the \a transaction.
		bool verify(const model::Transaction& transaction) const;

	private:
		GenerationHashSeed m_generationHashSeed;
	};
}}
