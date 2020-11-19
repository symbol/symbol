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

#include "MongoErrorPolicy.h"
#include "MongoBulkWriter.h"
#include "mappers/MapperUtils.h"
#include "catapult/exceptions.h"

namespace catapult { namespace mongo {

	namespace {
		bool CheckExact(uint64_t numExpected, uint64_t numActual, MongoErrorPolicy::Mode mode) {
			return numExpected == numActual || (MongoErrorPolicy::Mode::Idempotent == mode && numExpected >= numActual);
		}
	}

	MongoErrorPolicy::MongoErrorPolicy(const std::string& collectionName, Mode mode)
			: m_collectionName(collectionName)
			, m_mode(mode)
	{}

	MongoErrorPolicy::Mode MongoErrorPolicy::mode() const {
		return m_mode;
	}

	void MongoErrorPolicy::checkDeleted(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const {
		auto numActual = mappers::ToUint32(result.NumDeleted);
		if (CheckExact(numExpected, numActual, m_mode))
			return;

		formatMessageAndThrow("deleting", numExpected, numActual, itemsDescription);
	}

	void MongoErrorPolicy::checkDeletedAtLeast(
			uint64_t numExpected,
			const BulkWriteResult& result,
			const std::string& itemsDescription) const {
		if (Mode::Idempotent == m_mode)
			return;

		auto numActual = mappers::ToUint32(result.NumDeleted);
		if (numExpected <= numActual)
			return;

		formatMessageAndThrow("deleting (at least)", numExpected, numActual, itemsDescription);
	}

	void MongoErrorPolicy::checkInserted(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const {
		auto numActual = mappers::ToUint32(result.NumInserted);
		if (CheckExact(numExpected, numActual, m_mode))
			return;

		formatMessageAndThrow("inserting", numExpected, numActual, itemsDescription);
	}

	void MongoErrorPolicy::checkUpserted(uint64_t numExpected, const BulkWriteResult& result, const std::string& itemsDescription) const {
		auto numActual = mappers::ToUint32(result.NumModified) + mappers::ToUint32(result.NumUpserted);
		if (CheckExact(numExpected, numActual, m_mode))
			return;

		formatMessageAndThrow("upserting", numExpected, numActual, itemsDescription);
	}

	void MongoErrorPolicy::formatMessageAndThrow(
			const char* operation,
			uint64_t numExpected,
			uint64_t numActual,
			const std::string& itemsDescription) const {
		std::ostringstream out;
		out << "error " << operation << " " << itemsDescription;

		if (!m_collectionName.empty())
			out << " from [" << m_collectionName << "] collection";

		out << " (" << numExpected << " expected, " << numActual << " actual)";
		CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
	}
}}
