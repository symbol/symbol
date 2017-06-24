#pragma once
#include "Casting.h"
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace utils {

	/// Represents a file size.
	class FileSize final {
	private:
		constexpr explicit FileSize(uint64_t bytes) : m_bytes(bytes)
		{}

	public:
		/// Creates a default (zero) file size.
		constexpr FileSize() : FileSize(0)
		{}

	public:
		/// Creates a file size from the given number of \a megabytes.
		static constexpr FileSize FromMegabytes(uint64_t megabytes) {
			return FromKilobytes(megabytes * 1024);
		}

		/// Creates a file size from the given number of \a kilobytes.
		static constexpr FileSize FromKilobytes(uint64_t kilobytes) {
			return FromBytes(kilobytes * 1024);
		}

		/// Creates a file size from the given number of \a bytes.
		static constexpr FileSize FromBytes(uint64_t bytes) {
			return FileSize(bytes);
		}

	public:
		/// Returns the number of megabytes.
		constexpr uint64_t megabytes() const {
			return kilobytes() / 1024;
		}

		/// Returns the number of kilobytes.
		constexpr uint64_t kilobytes() const {
			return bytes() / 1024;
		}

		/// Returns the number of bytes.
		constexpr uint64_t bytes() const {
			return m_bytes;
		}

		/// Returns the number of bytes as a uint32_t.
		uint32_t bytes32() const {
			return checked_cast<uint64_t, uint32_t>(m_bytes);
		}

	public:
		/// Returns \c true if this file size is equal to \a rhs.
		constexpr bool operator==(const FileSize& rhs) const {
			return m_bytes == rhs.m_bytes;
		}

		/// Returns \c true if this file size is not equal to \a rhs.
		constexpr bool operator!=(const FileSize& rhs) const {
			return !(*this == rhs);
		}

		/// Returns \c true if this file size is greater than or equal to \a rhs.
		constexpr bool operator>=(const FileSize& rhs) const {
			return m_bytes >= rhs.m_bytes;
		}

		/// Returns \c true if this file size is greater than \a rhs.
		constexpr bool operator>(const FileSize& rhs) const {
			return m_bytes > rhs.m_bytes;
		}

		/// Returns \c true if this file size is less than or equal to \a rhs.
		constexpr bool operator<=(const FileSize& rhs) const {
			return m_bytes <= rhs.m_bytes;
		}

		/// Returns \c true if this file size is less than \a rhs.
		constexpr bool operator<(const FileSize& rhs) const {
			return m_bytes < rhs.m_bytes;
		}

	private:
		uint64_t m_bytes;
	};

	/// Insertion operator for outputting \a fileSize to \a out.
	std::ostream& operator<<(std::ostream& out, const FileSize& fileSize);
}}
