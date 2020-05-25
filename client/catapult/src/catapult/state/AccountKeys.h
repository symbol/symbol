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
#include "catapult/utils/BitwiseEnum.h"
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace state {

	/// Container holding account key information.
	class AccountKeys {
	public:
		// region KeyType

		/// Types of account keys.
		enum class KeyType : uint8_t {
			/// Unset key.
			Unset = 0x00,

			/// Linked account public key.
			/// \note This can be either a remote or main account public key depending on context.
			Linked = 0x01,

			/// VRF public key.
			VRF = 0x02,

			/// Voting public key.
			Voting = 0x04,

			/// Node public key on which remote is allowed to harvest.
			Node = 0x08,

			/// All valid keys.
			All = Linked | VRF | Voting | Node
		};

		// endregion

		// region KeyAccessor

		/// Key accessor.
		template<typename TAccountPublicKey>
		class KeyAccessor {
		public:
			/// Creates unset key.
			KeyAccessor();

			/// Copy constructor that makes a deep copy of \a keyAccessor.
			KeyAccessor(const KeyAccessor& keyAccessor);

			/// Move constructor that move constructs a key accessor from \a keyAccessor.
			KeyAccessor(KeyAccessor&& keyAccessor);

		public:
			/// Assignment operator that makes a deep copy of \a keyAccessor.
			KeyAccessor& operator=(const KeyAccessor& keyAccessor);

			/// Move assignment operator that assigns \a keyAccessor.
			KeyAccessor& operator=(KeyAccessor&& keyAccessor);

		public:
			/// Returns \c true if the underlying key is set.
			explicit operator bool() const;

			/// Gets the underlying key or a zero key if unset.
			TAccountPublicKey get() const;

		public:
			/// Sets the underlying key to \a key.
			void set(const TAccountPublicKey& key);

			/// Unsets the underlying key.
			void unset();

		private:
			std::shared_ptr<TAccountPublicKey> m_pKey;
		};

		// endregion

	public:
		/// Gets the mask of set keys.
		KeyType mask() const;

		/// Gets the (const) linked public key.
		const KeyAccessor<Key>& linkedPublicKey() const;

		/// Gets the linked public key.
		KeyAccessor<Key>& linkedPublicKey();

		/// Gets the (const) vrf public key.
		const KeyAccessor<Key>& vrfPublicKey() const;

		/// Gets the vrf public key.
		KeyAccessor<Key>& vrfPublicKey();

		/// Gets the (const) voting public key.
		const KeyAccessor<VotingKey>& votingPublicKey() const;

		/// Gets the voting public key.
		KeyAccessor<VotingKey>& votingPublicKey();

		/// Gets the (const) node public key.
		const KeyAccessor<Key>& nodePublicKey() const;

		/// Gets the node public key.
		KeyAccessor<Key>& nodePublicKey();

	private:
		KeyAccessor<Key> m_linkedPublicKey;
		KeyAccessor<Key> m_vrfPublicKey;
		KeyAccessor<VotingKey> m_votingPublicKey;
		KeyAccessor<Key> m_nodePublicKey;
	};

	MAKE_BITWISE_ENUM(AccountKeys::KeyType)

	extern template class AccountKeys::KeyAccessor<Key>;
	extern template class AccountKeys::KeyAccessor<VotingKey>;
}}
