#include "RawBuffer.h"
#include <string.h>

namespace catapult { namespace utils {

	RawString::RawString(const char* str) : RawString(str, strlen(str))
	{}

	MutableRawString::MutableRawString(std::string& str) : MutableRawString(&str[0], str.size())
	{}
}}
