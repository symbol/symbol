#ifdef DEFINE_ENUM
#define ENUM_TYPE DEFINE_ENUM
#endif

#if !defined(ENUM_LIST) || !defined(ENUM_TYPE)
#error "In order to use MacroBasedEnum.h ENUM_LIST and DEFINE_ENUM must be defined"
#endif

#ifdef DEFINE_ENUM
#define CONCAT_SYMBOLS(LEFT, RIGHT) LEFT##RIGHT
#define QUOTE(NAME) #NAME
#define STR(MACRO) QUOTE(MACRO)

	namespace {
		const char* CONCAT_SYMBOLS(ENUM_TYPE, ToString)(ENUM_TYPE value) {
			switch (value) {
#ifdef EXPLICIT_VALUE_ENUM
#define ENUM_VALUE(LABEL, VALUE) case ENUM_TYPE::LABEL: return #LABEL;
#else
#define ENUM_VALUE(LABEL) case ENUM_TYPE::LABEL: return #LABEL;
#endif

			ENUM_LIST

#undef ENUM_VALUE
			}

			return nullptr;
		}
	}

	std::ostream& operator<<(std::ostream& out, ENUM_TYPE value) {
		auto pLabel = CONCAT_SYMBOLS(ENUM_TYPE, ToString)(value);
		if (pLabel)
			out << pLabel;
		else
			out << STR(ENUM_TYPE) << "(0x" << utils::HexFormat(utils::to_underlying_type(value)) << ")";
		return out;
	}

#undef STR
#undef QUOTE
#undef CONCAT_SYMBOLS
#endif

#undef ENUM_TYPE
