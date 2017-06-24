#pragma once
#include <string>

namespace catapult { namespace utils {

	/// A diagnostic counter id.
	class DiagnosticCounterId {
	public:
		/// The maximum counter name size.
		static constexpr auto Max_Counter_Name_Size = 13u;

	public:
		/// Creates an empty id.
		DiagnosticCounterId();

		/// Creates an id from \a name.
		explicit DiagnosticCounterId(const std::string& name);

		/// Creates an id from \a value.
		explicit DiagnosticCounterId(uint64_t value);

	public:
		/// Gets the id name.
		const std::string& name() const {
			return m_name;
		}

		/// Gets the id value.
		uint64_t value() const {
			return m_value;
		}

	private:
		std::string m_name;
		uint64_t m_value;
	};
}}
