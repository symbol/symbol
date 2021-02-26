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
#include "catapult/model/NetworkIdentifier.h"
#include <list>

namespace catapult { namespace tools { namespace addressgen {

	/// Collection of address search patterns that are used to match against private keys.
	class MultiAddressMatcher {
	public:
		/// Creates a matcher for the network identified by \a networkIdentifier.
		explicit MultiAddressMatcher(model::NetworkIdentifier networkIdentifier);

	public:
		/// Returns \c true when all search patterns have been matched.
		bool isComplete() const;

	public:
		/// Adds a new search \a pattern.
		void addSearchPattern(const std::string& pattern);

		/// Attempts to match \a candidatePrivateKey against a preregistered search pattern.
		const crypto::KeyPair* accept(crypto::PrivateKey&& candidatePrivateKey);

	private:
		struct CandidateDescriptor {
		public:
			CandidateDescriptor(model::NetworkIdentifier networkIdentifier, crypto::PrivateKey&& privateKey);

		public:
			crypto::KeyPair KeyPair;
			std::string AddressString;
		};

		struct SearchDescriptor {
		public:
			bool IsComplete() const;

		public:
			std::string SearchString;

			bool MatchStart = false;
			bool MatchEnd = false;
			size_t BestMatchSize = 0;

			std::unique_ptr<crypto::KeyPair> pBestKeyPair;
		};

	private:
		model::NetworkIdentifier m_networkIdentifier;
		std::list<SearchDescriptor> m_descriptors;
	};
}}}
