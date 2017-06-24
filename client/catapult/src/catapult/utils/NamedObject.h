#pragma once
#include "catapult/preprocessor.h"
#include <string>
#include <vector>

namespace catapult { namespace utils {

	/// Mixin to have named objects.
	class NamedObjectMixin {
	public:
		/// Creates a mixin with \a name.
		explicit NamedObjectMixin(const std::string& name) : m_name(name)
		{}

	public:
		/// Returns the name.
		const std::string& name() const {
			return m_name;
		}

	private:
		std::string m_name;
	};

	/// Extracts all names from \a namedObjects.
	template<typename TNamedObjects>
	std::vector<std::string> ExtractNames(const TNamedObjects& namedObjects) {
		std::vector<std::string> names;
		names.reserve(namedObjects.size());
		for (const auto& pObject : namedObjects)
			names.push_back(pObject->name());

		return names;
	}

	/// Reduces \a names into a single string.
	CATAPULT_INLINE
	std::string ReduceNames(const std::vector<std::string>& names) {
		size_t numNames = 0;
		std::string result = "{";
		for (const auto& name : names) {
			if (0 != numNames++)
				result += ",";

			result += " ";
			result += name;
		}

		if (0 != numNames)
			result += " ";

		result += "}";
		return result;
	}
}}
