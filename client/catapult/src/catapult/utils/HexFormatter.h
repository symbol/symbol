#pragma once
#include "traits/Traits.h"
#include "catapult/preprocessor.h"
#include <array>
#include <iomanip>
#include <iostream>
#include <vector>

namespace catapult { namespace utils {

	/// RAII class that configures a stream to print out integral hex numbers.
	/// \note stream width is not sticky so it doesn't need to be saved / restored even though it is modified.
	template<size_t N>
	class IntegralHexFormatterGuard {
	public:
		explicit IntegralHexFormatterGuard(std::ostream& out) :
				m_out(out),
				m_flags(m_out.flags(std::ios::hex | std::ios::uppercase)),
				m_fill(m_out.fill('0'))
		{}

		~IntegralHexFormatterGuard() {
			m_out.flags(m_flags);
			m_out.fill(m_fill);
		}

	public:
		template<typename T>
		void output(T value) {
			Output(m_out, value);
		}

	private:
		template<typename T>
		static typename std::enable_if<std::is_integral<T>::value>::type Output(std::ostream& out, T value) {
			out << std::setw(2 * sizeof(T)) << +value;
		}

		template<typename T>
		static typename std::enable_if<!std::is_integral<T>::value>::type Output(std::ostream& out, T value) {
			auto pData = reinterpret_cast<const uint8_t*>(&value);
			for (auto i = 0u; i < sizeof(T); ++i)
				Output(out, pData[sizeof(T) - 1 - i]);
		}

	private:
		std::ostream& m_out;
		std::ios_base::fmtflags m_flags;
		char m_fill;
	};

	/// Formatter for printing an integral hex number to a stream.
	template<typename T, size_t N = sizeof(T)>
	class IntegralHexFormatter {
	public:
		explicit IntegralHexFormatter(T value) : m_value(value)
		{}

	public:
		friend std::ostream& operator<<(std::ostream& out, const IntegralHexFormatter<T>& formatter) {
			IntegralHexFormatterGuard<N> guard(out);
			guard.output(formatter.m_value);
			return out;
		}

	private:
		T m_value;
	};

	/// Factory function for creating a hex formatter around \a value.
	template<typename T, typename X = typename std::enable_if<traits::is_scalar<T>::value>::type>
	auto HexFormat(T value) {
		return IntegralHexFormatter<T>(value);
	}

	/// Formatter for printing a container of integral hex numbers to a stream.
	template<typename TInputIterator>
	class ContainerHexFormatter {
	public:
		ContainerHexFormatter(TInputIterator begin, TInputIterator end, char separator) :
				m_begin(begin),
				m_end(end),
				m_separator(separator)
		{}

	public:
		friend std::ostream& operator<<(std::ostream& out, const ContainerHexFormatter<TInputIterator>& formatter) {
			using T = decltype(*m_begin);
			IntegralHexFormatterGuard<sizeof(T)> guard(out);
			for (auto iter = formatter.m_begin; formatter.m_end != iter; ++iter) {
				guard.output(*iter);

				if (0 != formatter.m_separator && iter + 1 != formatter.m_end)
					out << formatter.m_separator;
			}

			return out;
		}

	private:
		TInputIterator m_begin;
		TInputIterator m_end;
		char m_separator;
	};

	/// Factory function for creating a hex formatter around iterators \a begin and \a end with \a separator.
	template<typename TInputIterator>
	auto HexFormat(TInputIterator begin, TInputIterator end, char separator = 0) {
		return ContainerHexFormatter<TInputIterator>(begin, end, separator);
	}

	/// Factory function for creating a hex formatter around \a container with \a separator.
	template<typename TContainer>
	auto HexFormat(const TContainer& container, char separator) {
		return HexFormat(std::cbegin(container), std::cend(container), separator);
	}

	/// Factory function for creating a hex formatter around \a data.
	template<typename T, typename X = typename std::enable_if<!traits::is_scalar<T>::value>::type>
	auto HexFormat(const T& data) {
		auto pData = reinterpret_cast<const uint8_t*>(&data);
		return HexFormat(pData, pData + sizeof(T), 0);
	}

	/// Factory function for creating a hex formatter around \a container.
	template<typename T>
	auto HexFormat(const std::vector<T>& container) {
		return HexFormat(container, 0);
	}

	/// Factory function for creating a hex formatter around \a container.
	template<typename T, size_t N>
	auto HexFormat(const std::array<T, N>& container) {
		return HexFormat(container, 0);
	}
}}
