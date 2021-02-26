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

#include "MultiAddressMatcher.h"
#include "catapult/model/Address.h"

namespace catapult { namespace tools { namespace addressgen {

	namespace {
		void PopFront(std::string& str) {
			str = str.substr(1);
		}
	}

	MultiAddressMatcher::MultiAddressMatcher(model::NetworkIdentifier networkIdentifier) : m_networkIdentifier(networkIdentifier)
	{}

	bool MultiAddressMatcher::isComplete() const {
		return std::all_of(m_descriptors.cbegin(), m_descriptors.cend(), [](const auto& descriptor) {
			return descriptor.IsComplete();
		});
	}

	void MultiAddressMatcher::addSearchPattern(const std::string& pattern) {
		SearchDescriptor descriptor;
		descriptor.SearchString = pattern;

		if ('^' == pattern[0]) {
			descriptor.MatchStart = true;
			PopFront(descriptor.SearchString);
		} else if ('$' == pattern.back()) {
			descriptor.MatchEnd = true;
			descriptor.SearchString.pop_back();
		}

		m_descriptors.push_back(std::move(descriptor));
	}

	const crypto::KeyPair* MultiAddressMatcher::accept(crypto::PrivateKey&& candidatePrivateKey) {
		auto candidateDescriptor = CandidateDescriptor(m_networkIdentifier, std::move(candidatePrivateKey));
		const auto& addressString = candidateDescriptor.AddressString;

		for (auto& descriptor : m_descriptors) {
			auto searchString = descriptor.SearchString;
			while (!searchString.empty() && (searchString.size() > descriptor.BestMatchSize || !descriptor.pBestKeyPair)) {
				auto matchIndex = addressString.find(searchString);
				auto isMatch = std::string::npos != matchIndex;
				if (descriptor.MatchStart)
					isMatch = 0 == matchIndex;
				else if (descriptor.MatchEnd)
					isMatch = matchIndex + searchString.size() == addressString.size();

				if (isMatch) {
					if (!searchString.empty()) {
						CATAPULT_LOG(info)
								<< "searching for '" << descriptor.SearchString << "' found " << addressString
								<< " (" << searchString.size() << "/" << descriptor.SearchString.size() << ")";
					}

					descriptor.BestMatchSize = searchString.size();
					descriptor.pBestKeyPair = std::make_unique<crypto::KeyPair>(std::move(candidateDescriptor.KeyPair));

					if (descriptor.IsComplete())
						return descriptor.pBestKeyPair.get();
				}

				if (descriptor.MatchEnd)
					PopFront(searchString);
				else
					searchString.pop_back();
			}
		}

		return nullptr;
	}

	MultiAddressMatcher::CandidateDescriptor::CandidateDescriptor(
			model::NetworkIdentifier networkIdentifier,
			crypto::PrivateKey&& privateKey)
			: KeyPair(crypto::KeyPair::FromPrivate(std::move(privateKey)))
			, AddressString(model::AddressToString(model::PublicKeyToAddress(KeyPair.publicKey(), networkIdentifier)))
	{}

	bool MultiAddressMatcher::SearchDescriptor::IsComplete() const {
		return SearchString.size() == BestMatchSize && !!pBestKeyPair;
	}
}}}
