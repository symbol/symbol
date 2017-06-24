#pragma once
#include "ConfigurationValueParsers.h"
#include "catapult/exceptions.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace utils {

	/// Exception class that is thrown when a required configuration property is missing.
	class VISIBLE_EXCEPTION_ATTRIBUTE property_not_found_error : public catapult_invalid_argument {
	public:
		using catapult_invalid_argument::catapult_invalid_argument;
	};

	/// Exception class that is thrown when a configuration property is malformed.
	class VISIBLE_EXCEPTION_ATTRIBUTE property_malformed_error : public catapult_runtime_error {
	public:
		using catapult_runtime_error::catapult_runtime_error;
	};

	/// A configuration key.
	struct ConfigurationKey {
		/// Creates a configuration key for a key with \a name in \a section.
		constexpr ConfigurationKey(const char* section, const char* name) : Section(section), Name(name)
		{}

		/// The section containing the key.
		const char* Section;

		/// The key name.
		const char* Name;
	};

	/// A simple bag of properties.
	class ConfigurationBag {
	public:
		/// A strongly typed key to value map.
		template<typename TValue>
		using KeyValueMap = std::unordered_map<std::string, TValue>;

		/// The type of container that a bag is created around.
		using ValuesContainer = std::unordered_map<std::string, KeyValueMap<std::string>>;

	public:
		/// Creates a new configuration bag around \a values.
		ConfigurationBag(ValuesContainer&& values) : m_values(values)
		{}

	public:
		/// Loads a configuration bag from the specified stream (\a input).
		static ConfigurationBag FromStream(std::istream& input);

		/// Loads a configuration bag from the specified \a path.
		static ConfigurationBag FromPath(const std::string& path);

	public:
		/// Returns the number of properties in this bag.
		size_t size() const {
			size_t count = 0;
			for (const auto& section : m_values)
				count += section.second.size();

			return count;
		}

		/// Returns the number of \a section properties in this bag.
		size_t size(const char* section) const {
			auto sectionIter = m_values.find(section);
			return m_values.cend() == sectionIter ? 0 : sectionIter->second.size();
		}

		/// Returns \c true if the property identified by \a key is contained in this bag.
		bool contains(const ConfigurationKey& key) const {
			return nullptr != lookup(key);
		}

		/// Gets the names of all sections in this bag.
		std::unordered_set<std::string> sections() const {
			std::unordered_set<std::string> sections;
			for (const auto& pair : m_values)
				sections.insert(pair.first);

			return sections;
		}

		/// Tries to get the property identified by \a key from this bag and puts it into \a value.
		/// \note this function will throw if the value is present but malformed.
		template<typename T>
		bool tryGet(const ConfigurationKey& key, T& value) const {
			auto pUnparsedValue = lookup(key);
			if (nullptr == pUnparsedValue)
				return false;

			if (!TryParseValue(*pUnparsedValue, value)) {
				auto message = "property could not be parsed";
				CATAPULT_THROW_AND_LOG_2(property_malformed_error, message, std::string(key.Section), std::string(key.Name));
			}

			return true;
		}

		/// Gets the property identified by \a key from this bag.
		template<typename T>
		T get(const ConfigurationKey& key) const {
			T value;
			if (!tryGet(key, value)) {
				auto message = "property not found";
				CATAPULT_THROW_AND_LOG_2(property_not_found_error, message, std::string(key.Section), std::string(key.Name));
			}

			return value;
		}

		/// Gets all \a section properties from this bag.
		template<typename T>
		KeyValueMap<T> getAll(const char* section) const {
			KeyValueMap<T> values;

			auto sectionIter = m_values.find(section);
			if (m_values.cend() == sectionIter)
				return values;

			for (const auto& pair : sectionIter->second)
				values.emplace(pair.first, get<T>(ConfigurationKey(section, pair.first.c_str())));

			return values;
		}

	private:
		const std::string* lookup(const ConfigurationKey& key) const {
			auto sectionIter = m_values.find(key.Section);
			if (m_values.cend() == sectionIter)
				return nullptr;

			const auto& sectionValues = sectionIter->second;
			auto itemIter = sectionValues.find(key.Name);
			if (sectionValues.cend() == itemIter)
				return nullptr;

			return &itemIter->second;
		}

	private:
		ValuesContainer m_values;
	};
}}
