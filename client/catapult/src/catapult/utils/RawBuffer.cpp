#include "RawBuffer.h"
#include <string.h>

namespace catapult { namespace utils {

	RawString::RawString(const char* pStr) : RawString(pStr, strlen(pStr))
	{}

	MutableRawString::MutableRawString(std::string& str) : MutableRawString(&str[0], str.size())
	{}
}}
