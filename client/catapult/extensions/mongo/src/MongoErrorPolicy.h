/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/utils/NonCopyable.h"
#include <string>

namespace catapult { namespace mongo { struct BulkWriteResult; } }

namespace catapult { namespace mongo {

	/// Error policy for checking mongo operation results.
	class MongoErrorPolicy : utils::MoveOnly {
	public:
		/// Error policy modes.
		enum class Mode {
			/// Strictest mode that requires exact matching.
			Strict,

			/// More forgiving mode that enables idempotent operations.
			/// \note This is recommended for recovery.
			Idempotent
		};

	public:
		/// Creates an error policy around \a collectionName using error policy \a mode.
		MongoErrorPolicy(const std::string& collectionName, Mode mode);

	public:
		/// Gets the error policy mode.
		Mode mode() const;

	public:
		/// Checks that \a result indicates exactly \a numExpected deletions occurred given \a itemsDescription.
		void checkDeleted(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const;

		/// Checks that \a result indicates at least \a numExpected deletions occurred given \a itemsDescription.
		void checkDeletedAtLeast(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const;

		/// Checks that \a result indicates exactly \a numExpected insertions occurred given \a itemsDescription.
		void checkInserted(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const;

		/// Checks that \a result indicates exactly \a numExpected upsertions occurred given \a itemsDescription.
		void checkUpserted(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const;

	private:
		[[noreturn]]
		void formatMessageAndThrow(
				const char* operation,
				uint64_t numExpected,
				uint64_t numActual,
				const std::string& itemsDescription) const;

	private:
		std::string m_collectionName;
		Mode m_mode;
	};
}}
