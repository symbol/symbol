#include "Conversions.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/HexParser.h"
#include <sstream>

namespace catapult { namespace test {

	std::string ToHexString(const uint8_t* pData, size_t dataSize) {
		std::stringstream out;
		out << utils::HexFormat(pData, pData + dataSize);
		return out.str();
	}

	std::string ToHexString(const std::vector<uint8_t>& data) {
		return ToHexString(data.data(), data.size());
	}

	std::vector<uint8_t> ToVector(const std::string& hexString) {
		std::vector<uint8_t> result;
		if (hexString.size() % 2)
			CATAPULT_THROW_RUNTIME_ERROR_1("hexString has odd number of characters", hexString.size());

		result.resize(hexString.size() / 2);
		utils::ParseHexStringIntoContainer(hexString.c_str(), hexString.size(), result);
		return result;
	}
}}
