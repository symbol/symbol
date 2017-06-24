#pragma once

extern "C" {
	#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wreserved-id-macro"
	#pragma clang diagnostic ignored "-Wdocumentation"
	#endif

	#include <sha3/KeccakHash.h>

	#ifdef __clang__
	#pragma clang diagnostic pop
	#endif
}
