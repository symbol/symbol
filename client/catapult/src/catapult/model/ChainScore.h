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
#include "catapult/utils/BaseValue.h"
#include "catapult/functions.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <iosfwd>

namespace catapult { namespace model {

	/// Score of a block chain.
	/// \note This is a 128-bit value.
	class ChainScore {
	private:
		using ArrayType = std::array<uint64_t, 2>;
		static constexpr uint32_t Bits_Per_Value = 64;

	public:
		struct Delta_tag {};
		using Delta = utils::BaseValue<int64_t, Delta_tag>;

	public:
		/// Creates a default chain score.
		ChainScore() : ChainScore(0)
		{}

		/// Creates a chain score from a 64-bit value (\a score).
		explicit ChainScore(uint64_t score) : m_score(score)
		{}

		/// Creates a chain score from a 128-bit value composed of two 64-bit values (\a scoreHigh and \a scoreLow).
		ChainScore(uint64_t scoreHigh, uint64_t scoreLow) {
			m_score = scoreHigh;
			m_score <<= Bits_Per_Value;
			m_score += scoreLow;
		}

	public:
		ChainScore(const ChainScore&) = default;
		ChainScore& operator=(const ChainScore&) = default;

	public:
		/// Gets an array representing the underlying score.
		ArrayType toArray() const {
			return {{
				static_cast<uint64_t>(m_score >> Bits_Per_Value),
				static_cast<uint64_t>(m_score & std::numeric_limits<uint64_t>::max())
			}};
		}

	public:
		/// Adds \a rhs to this chain score.
		ChainScore& operator+=(const ChainScore& rhs) {
			m_score += rhs.m_score;
			return *this;
		}

		/// Adds \a rhs to this chain score.
		ChainScore& operator+=(Delta rhs) {
			if (rhs < Delta())
				m_score -= static_cast<uint64_t>(-rhs.unwrap());
			else
				m_score += rhs.unwrap();

			return *this;
		}

		/// Subtracts \a rhs from this chain score.
		Delta operator-(const ChainScore& rhs) {
			boost::multiprecision::checked_uint128_t result = rhs < *this
					? m_score - rhs.m_score
					: rhs.m_score - m_score;

			if (result > std::numeric_limits<int64_t>::max())
				throw std::range_error("subtraction failed because chain scores are too far apart");

			return rhs < *this
					? Delta(static_cast<int64_t>(result))
					: Delta(-static_cast<int64_t>(result));
		}

	public:
		/// Returns \c true if this score is equal to \a rhs.
		bool operator==(const ChainScore& rhs) const {
			return m_score == rhs.m_score;
		}

		/// Returns \c true if this score is not equal to \a rhs.
		bool operator!=(const ChainScore& rhs) const {
			return m_score != rhs.m_score;
		}

		/// Returns \c true if this score is less than \a rhs.
		bool operator<(const ChainScore& rhs) const {
			return m_score < rhs.m_score;
		}

		/// Returns \c true if this score is less than or equal to \a rhs.
		bool operator<=(const ChainScore& rhs) const {
			return m_score <= rhs.m_score;
		}

		/// Returns \c true if this score is greater than \a rhs.
		bool operator>(const ChainScore& rhs) const {
			return m_score > rhs.m_score;
		}

		/// Returns \c true if this score is greater than or equal to \a rhs.
		bool operator>=(const ChainScore& rhs) const {
			return m_score >= rhs.m_score;
		}

	public:
		/// Insertion operator for outputting \a score to \a out.
		friend std::ostream& operator<<(std::ostream& out, const ChainScore& score) {
			out << score.m_score;
			return out;
		}

	private:
		boost::multiprecision::checked_uint128_t m_score;
	};

	/// Prototype for a function that returns a chain score.
	using ChainScoreSupplier = supplier<ChainScore>;
}}
