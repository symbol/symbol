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
