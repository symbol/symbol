#pragma once

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wdocumentation"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4324)
#endif

extern "C" {
#include <sha3/KeccakHash.h>
}

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
