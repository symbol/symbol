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
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Runs a roundtrip test by serializing \a value using \a serializer and then deserializing it using \a deserializer into \a result.
	template<typename TValue, typename TResultValue, typename TSerializer, typename TDeserializer>
	void RunRoundtripBufferTest(const TValue& value, TResultValue& result, TSerializer serializer, TDeserializer deserializer) {
		// Act: serialize it
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);
		serializer(value, outputStream);

		// - deserialize it
		mocks::MockMemoryStream inputStream(buffer);
		deserializer(inputStream, result);

		// Assert: whole buffer has been read
		EXPECT_EQ(buffer.size(), inputStream.position());
	}

	/// Runs a roundtrip test by serializing \a value using \a serializer and then deserializing it using \a deserializer.
	template<typename TValue, typename TSerializer, typename TDeserializer>
	auto RunRoundtripBufferTest(const TValue& value, TSerializer serializer, TDeserializer deserializer) {
		// Act: serialize it
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);
		serializer(value, outputStream);

		// - deserialize it
		mocks::MockMemoryStream inputStream(buffer);
		auto result = deserializer(inputStream);

		// Assert: whole buffer has been read
		EXPECT_EQ(buffer.size(), inputStream.position());
		return result;
	}

	/// Runs a roundtrip test by serializing \a value and then deserializing it.
	template<typename TSerializer, typename TValue>
	auto RunRoundtripBufferTest(const TValue& value) {
		return RunRoundtripBufferTest(value, TSerializer::Save, TSerializer::Load);
	}

	/// Runs a roundtrip test by serializing \a value to a string and then deserializing it.
	/// \note This function is intended for data stored directly in rocks db.
	template<typename TSerializer, typename TValue>
	TValue RunRoundtripStringTest(const TValue& value) {
		// Act: serialize it
		auto serializedString = TSerializer::SerializeValue(value);
		std::vector<uint8_t> serializedBuffer(serializedString.size());
		std::memcpy(serializedBuffer.data(), serializedString.data(), serializedString.size());

		// - deserialize it
		return TSerializer::DeserializeValue(serializedBuffer);
	}
}}
