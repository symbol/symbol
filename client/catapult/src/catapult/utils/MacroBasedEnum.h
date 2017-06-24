#ifdef DECLARE_ENUM
	#define ENUM_TYPE DECLARE_ENUM
#elif defined(DEFINE_ENUM)
	#define ENUM_TYPE DEFINE_ENUM
#endif

#if !defined(ENUM_LIST) || !defined(ENUM_TYPE)
#error "In order to use MacroBasedEnum.h ENUM_LIST and one of { DECLARE_ENUM, DEFINE_ENUM } must be defined"
#endif

#ifdef DECLARE_ENUM
	#ifdef EXPLICIT_TYPE_ENUM
	enum class ENUM_TYPE : EXPLICIT_TYPE_ENUM
	#else
	enum class ENUM_TYPE
	#endif
	{
		#ifdef EXPLICIT_VALUE_ENUM
		#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
		#else
		#define ENUM_VALUE(LABEL) LABEL,
		#endif

		ENUM_LIST

		#undef ENUM_VALUE
	};

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, ENUM_TYPE value);

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
