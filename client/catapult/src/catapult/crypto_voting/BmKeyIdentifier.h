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
#include "catapult/types.h"

namespace catapult { namespace crypto {

	/// Two-layer Bellare-Miner private key tree key identifier.
	struct BmKeyIdentifier {
	public:
		/// Invalid identifier.
		static constexpr uint64_t Invalid_Id = 0xFFFF'FFFF'FFFF'FFFF;

	public:
		/// Key id.
		uint64_t KeyId;

	public:
		/// Returns \c true if this key identifier is equal to \a rhs.
		bool operator==(const BmKeyIdentifier& rhs) const;

		/// Returns \c true if this key identifier is not equal to \a rhs.
		bool operator!=(const BmKeyIdentifier& rhs) const;

		/// Returns \c true if this key identifier is less than \a rhs.
		bool operator<(const BmKeyIdentifier& rhs) const;

		/// Returns \c true if this key identifier is less than or equal to \a rhs.
		bool operator<=(const BmKeyIdentifier& rhs) const;

		/// Returns \c true if this key identifier is greater than \a rhs.
		bool operator>(const BmKeyIdentifier& rhs) const;

		/// Returns \c true if this key identifier is greater than or equal to \a rhs.
		bool operator>=(const BmKeyIdentifier& rhs) const;
	};

	/// Insertion operator for outputting \a keyIdentifier to \a out.
	std::ostream& operator<<(std::ostream& out, const BmKeyIdentifier& keyIdentifier);
}}
