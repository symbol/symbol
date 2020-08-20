/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "StreamFormatGuard.h"
#include "traits/Traits.h"
#include <array>
#include <iostream>
#include <vector>

namespace catapult { namespace utils {

	/// RAII class that configures a stream to print out integral hex numbers.
	/// \note stream width is not sticky so it doesn't need to be saved / restored even though it is modified.
	template<size_t N>
	class IntegralHexFormatterGuard : public StreamFormatGuard {
	public:
		/// Creates a formatter guard around \a out.
		explicit IntegralHexFormatterGuard(std::ostream& out)
				: StreamFormatGuard(out, std::ios::hex | std::ios::uppercase, '0')
				, m_out(out)
		{}

	public:
		/// Outputs \a value to the underlying stream.
		template<typename T>
		void output(T value) {
			Output(m_out, value);
		}

	private:
		template<typename T>
		static void OutputValue(std::ostream& out, T value, int size) {
			out << std::setw(2 * size) << +value;
		}

		template<typename T>
		static void Output(std::ostream& out, const T& value) {
			if constexpr (std::is_integral_v<T>) {
				OutputValue(out, value, N);
			} else {
				const auto* pData = reinterpret_cast<const uint8_t*>(&value);
				for (auto i = 0u; i < sizeof(T); ++i)
					OutputValue(out, pData[sizeof(T) - 1 - i], 1);
			}
		}

	private:
		std::ostream& m_out;
	};

	/// Formatter for printing an integral hex number to a stream.
	template<typename T, size_t N = sizeof(T)>
	class IntegralHexFormatter {
	public:
		explicit IntegralHexFormatter(T value) : m_value(value)
		{}

	public:
		friend std::ostream& operator<<(std::ostream& out, const IntegralHexFormatter& formatter) {
			IntegralHexFormatterGuard<N> guard(out);
			guard.output(formatter.m_value);
			return out;
		}

	private:
		T m_value;
	};

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
	template<typename T>
	auto HexFormat(const T& data) {
		if constexpr (traits::is_scalar_v<T>) {
			return IntegralHexFormatter<T>(data);
		} else {
			const auto* pData = reinterpret_cast<const uint8_t*>(&data);
			return HexFormat(pData, pData + sizeof(T), 0);
		}
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
