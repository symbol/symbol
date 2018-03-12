#pragma once
#include "Casting.h"

/// Defines bitwise or operator for \a ENUM_TYPE.
#define BITWISE_ENUM_OPERATOR_OR(ENUM_TYPE) \
	constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) { \
		return static_cast<ENUM_TYPE>(utils::to_underlying_type(lhs) | utils::to_underlying_type(rhs)); \
	}

/// Defines bitwise or assignment operator for \a ENUM_TYPE.
#define BITWISE_ENUM_OPERATOR_OR_EQUALS(ENUM_TYPE) \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) { \
		return lhs = lhs | rhs; \
	}

/// Defines HasFlags helper for \a ENUM_TYPE.
#define BITWISE_ENUM_HAS_FLAG_HELPER(ENUM_TYPE) \
	constexpr bool HasFlag(ENUM_TYPE testedFlag, ENUM_TYPE value) { \
		return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value)); \
	}

/// Defines bitwise helpers for \a ENUM_TYPE.
#define MAKE_BITWISE_ENUM(ENUM_TYPE) \
	BITWISE_ENUM_OPERATOR_OR(ENUM_TYPE) \
	BITWISE_ENUM_OPERATOR_OR_EQUALS(ENUM_TYPE) \
	BITWISE_ENUM_HAS_FLAG_HELPER(ENUM_TYPE)
