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

#include "BmKeyIdentifier.h"

namespace catapult { namespace crypto {

	namespace {
		bool IsIdLessThan(uint64_t lhs, uint64_t rhs) {
			if (lhs == rhs || BmKeyIdentifier::Invalid_Id == rhs)
				return false;

			return BmKeyIdentifier::Invalid_Id == lhs || lhs < rhs;
		}
	}

	bool BmKeyIdentifier::operator==(const BmKeyIdentifier& rhs) const {
		return BatchId == rhs.BatchId && KeyId == rhs.KeyId;
	}

	bool BmKeyIdentifier::operator!=(const BmKeyIdentifier& rhs) const {
		return !(*this == rhs);
	}

	bool BmKeyIdentifier::operator<(const BmKeyIdentifier& rhs) const {
		return IsIdLessThan(BatchId, rhs.BatchId) || (BatchId == rhs.BatchId && IsIdLessThan(KeyId, rhs.KeyId));
	}

	bool BmKeyIdentifier::operator<=(const BmKeyIdentifier& rhs) const {
		return *this < rhs || *this == rhs;
	}

	bool BmKeyIdentifier::operator>(const BmKeyIdentifier& rhs) const {
		return !(*this <= rhs);
	}

	bool BmKeyIdentifier::operator>=(const BmKeyIdentifier& rhs) const {
		return !(*this < rhs);
	}

	std::ostream& operator<<(std::ostream& out, const BmKeyIdentifier& keyIdentifier) {
		out << "(" << keyIdentifier.BatchId << ", " << keyIdentifier.KeyId << ")";
		return out;
	}
}}
