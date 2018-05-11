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

#include "ConsumerInput.h"
#include "catapult/utils/HexFormatter.h"
#include <ostream>

namespace catapult { namespace disruptor {

	namespace {
		void OutputShortHash(std::ostream& out, const Hash256& hash) {
			out << " [" << utils::HexFormat(hash.data(), hash.data() + 4) << "] ";
		}

		void OutputElementsInfo(std::ostream& out, const BlockElements& elements) {
			out << " (heights " << elements.front().Block.Height << " - " << elements.back().Block.Height << ")";
		}

		void OutputElementsInfo(std::ostream&, const TransactionElements&) {
		}

		template<typename TElements>
		void OutputElementsInfoT(std::ostream& out, const TElements& elements, const char* tag) {
			if (elements.empty())
				return;

			out << elements.size() << " " << tag;
			OutputElementsInfo(out, elements);
			OutputShortHash(out, elements[0].EntityHash);
		}
	}

	std::ostream& operator<<(std::ostream& out, const ConsumerInput& input) {
		OutputElementsInfoT(out, input.m_blockElements, "blocks");
		OutputElementsInfoT(out, input.m_transactionElements, "txes");

		if (input.empty())
			out << "empty ";

		out << "from " << input.source();
		return out;
	}
}}
