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
#include "catapult/model/PinnedVotingKey.h"
#include "catapult/utils/BitwiseEnum.h"
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace state {

	/// Container holding supplemental account public key information.
	class AccountPublicKeys {
	public:
		// region KeyType

		/// Types of account public keys.
		enum class KeyType : uint8_t {
			/// Unset key.
			Unset = 0x00,

			/// Linked account public key.
			/// \note This can be either a remote or main account public key depending on context.
			Linked = 0x01,

			/// Node public key on which remote is allowed to harvest.
			Node = 0x02,

			/// VRF public key.
			VRF = 0x04,

			/// All valid keys.
			All = Linked | Node | VRF
		};

		// endregion

		// region PublicKeyAccessor

		/// Accessor for a single public key.
		template<typename TAccountPublicKey>
		class PublicKeyAccessor {
		public:
			/// Creates unset key.
			PublicKeyAccessor();

			/// Copy constructor that makes a deep copy of \a accessor.
			PublicKeyAccessor(const PublicKeyAccessor& accessor);

			/// Move constructor that move constructs a key accessor from \a accessor.
			PublicKeyAccessor(PublicKeyAccessor&& accessor);

		public:
			/// Assignment operator that makes a deep copy of \a accessor.
			PublicKeyAccessor& operator=(const PublicKeyAccessor& accessor);

			/// Move assignment operator that assigns \a accessor.
			PublicKeyAccessor& operator=(PublicKeyAccessor&& accessor);

		public:
			/// Returns \c true if the underlying public key is set.
			explicit operator bool() const;

			/// Gets the underlying public key or a zero key if unset.
			TAccountPublicKey get() const;

		public:
			/// Sets the underlying public key to \a key.
			void set(const TAccountPublicKey& key);

			/// Unsets the underlying public key.
			void unset();

		private:
			std::shared_ptr<TAccountPublicKey> m_pKey;
		};

		// endregion

		// region PublicKeysAccessor

		/// Accessor for multiple (pinned) public keys.
		template<typename TPinnedAccountPublicKey>
		class PublicKeysAccessor {
		private:
			using const_iterator = typename std::vector<TPinnedAccountPublicKey>::const_iterator;

		public:
			/// Creates unset key.
			PublicKeysAccessor();

			/// Copy constructor that makes a deep copy of \a accessor.
			PublicKeysAccessor(const PublicKeysAccessor& accessor);

			/// Move constructor that move constructs a key accessor from \a accessor.
			PublicKeysAccessor(PublicKeysAccessor&& accessor);

		public:
			/// Assignment operator that makes a deep copy of \a accessor.
			PublicKeysAccessor& operator=(const PublicKeysAccessor& accessor);

			/// Move assignment operator that assigns \a accessor.
			PublicKeysAccessor& operator=(PublicKeysAccessor&& accessor);

		public:
			/// Gets the number of public keys
			size_t size() const;

			/// Gets the largest finalization point for which a public key is associated.
			FinalizationPoint upperBound() const;

			/// Returns \c true if a public key is associated with \a point.
			std::pair<size_t, bool> contains(FinalizationPoint point) const;

			/// Returns \c true if the specified public \a key is exactly contained.
			bool containsExact(const TPinnedAccountPublicKey& key) const;

			/// Gets the public key at \a index.
			const TPinnedAccountPublicKey& get(size_t index) const;

			/// Gets all public keys.
			std::vector<TPinnedAccountPublicKey> getAll() const;

		public:
			/// Adds the specified public \a key to the container.
			void add(const TPinnedAccountPublicKey& key);

			/// Removes the underlying public key matching \a key.
			bool remove(const TPinnedAccountPublicKey& key);

		private:
			const_iterator findExact(const TPinnedAccountPublicKey& key) const;

		private:
			std::shared_ptr<std::vector<TPinnedAccountPublicKey>> m_pKeys;
		};

		// endregion

	public:
		/// Gets the mask of set (non-voting) public keys.
		KeyType mask() const;

		/// Gets the (const) linked public key accessor.
		const PublicKeyAccessor<Key>& linked() const;

		/// Gets the linked public key accessor.
		PublicKeyAccessor<Key>& linked();

		/// Gets the (const) node public key accessor.
		const PublicKeyAccessor<Key>& node() const;

		/// Gets the node public key accessor.
		PublicKeyAccessor<Key>& node();

		/// Gets the (const) vrf public key accessor.
		const PublicKeyAccessor<Key>& vrf() const;

		/// Gets the vrf public key accessor.
		PublicKeyAccessor<Key>& vrf();

		/// Gets the (const) voting public keys accessor.
		const PublicKeysAccessor<model::PinnedVotingKey>& voting() const;

		/// Gets the voting public keys accessor.
		PublicKeysAccessor<model::PinnedVotingKey>& voting();

	private:
		PublicKeyAccessor<Key> m_linkedPublicKeyAccessor;
		PublicKeyAccessor<Key> m_nodePublicKeyAccessor;
		PublicKeyAccessor<Key> m_vrfPublicKeyAccessor;
		PublicKeysAccessor<model::PinnedVotingKey> m_votingPublicKeysAccessor;
	};

	MAKE_BITWISE_ENUM(AccountPublicKeys::KeyType)

	extern template class AccountPublicKeys::PublicKeyAccessor<Key>;
	extern template class AccountPublicKeys::PublicKeysAccessor<model::PinnedVotingKey>;
}}
