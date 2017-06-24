#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <iosfwd>

namespace catapult { namespace model {

	/// The 128-bit score of a block chain.
	class ChainScore {
	private:
		using ArrayType = std::array<uint64_t, 2>;
		static constexpr uint32_t BitsPerValue() { return 64u; }

	public:
		/// Creates a default chain score.
		ChainScore() : ChainScore(0u)
		{}

		/// Creates a chain score from a 64-bit value (\a score).
		explicit ChainScore(uint64_t score) : m_score(score)
		{}

		/// Creates a chain score from a 128-bit value composed of two 64-bit values (\a scoreHigh and \a scoreLow).
		explicit ChainScore(uint64_t scoreHigh, uint64_t scoreLow) {
			m_score = scoreHigh;
			m_score <<= BitsPerValue();
			m_score += scoreLow;
		}

	public:
		ChainScore(const ChainScore&) = default;
		ChainScore& operator=(const ChainScore&) = default;

	public:
		/// Gets an array representing the underlying score.
		ArrayType toArray() const {
			return { {
				static_cast<uint64_t>(m_score >> BitsPerValue()),
				static_cast<uint64_t>(m_score & std::numeric_limits<uint64_t>::max())
			} };
		}

	public:
		/// Adds \a rhs to this chain score.
		ChainScore& operator+=(const ChainScore& rhs) {
			m_score += rhs.m_score;
			return *this;
		}

		/// Subtracts \a rhs from this chain score.
		ChainScore& operator-=(const ChainScore& rhs) {
			m_score -= rhs.m_score;
			return *this;
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
		boost::multiprecision::uint128_t m_score;
	};
}}
