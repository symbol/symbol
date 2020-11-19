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
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <type_traits>

namespace catapult {
	namespace model {
		struct EmbeddedTransaction;
		struct Receipt;
		struct VerifiableEntity;
	}
}

namespace catapult { namespace mongo { namespace mappers {

	namespace bson_stream {
		using bsoncxx::builder::stream::close_array;
		using bsoncxx::builder::stream::close_document;
		using bsoncxx::builder::stream::document;
		using bsoncxx::builder::stream::open_array;
		using bsoncxx::builder::stream::open_document;
		using bsoncxx::builder::stream::finalize;

		using array_context = bsoncxx::builder::stream::array_context<bsoncxx::builder::stream::key_context<>>;
	}

	// region conversions to db type

	/// Converts raw array (\a pData) of \a size elements into bson binary type.
	bsoncxx::v_noabi::types::b_binary ToBinary(const uint8_t* pData, size_t size);

	/// Converts \a unresolvedAddress into bson binary type.
	bsoncxx::v_noabi::types::b_binary ToBinary(const UnresolvedAddress& unresolvedAddress);

	/// Converts byte \a array into bson binary type.
	template<typename TTag>
	auto ToBinary(const utils::ByteArray<TTag>& array) {
		return ToBinary(array.data(), array.size());
	}

	/// Converts base \a value to int32_t.
	template<
		typename TBaseValue,
		typename X = std::enable_if_t<std::is_same_v<uint32_t, typename TBaseValue::ValueType>>>
	int32_t ToInt32(TBaseValue value) {
		return static_cast<int32_t>(value.unwrap());
	}

	/// Converts base \a value to int64_t.
	template<
		typename TBaseValue,
		typename X = std::enable_if_t<std::is_same_v<uint64_t, typename TBaseValue::ValueType>>>
	int64_t ToInt64(TBaseValue value) {
		return static_cast<int64_t>(value.unwrap());
	}

	// endregion

	// region conversions from db type

	/// Converts a 32 bit signed \a value to an 8 bit unsigned value.
	uint8_t ToUint8(int32_t value);

	/// Converts a 32 bit signed \a value to a 32 bit unsigned value.
	uint32_t ToUint32(int32_t value);

	/// Gets a 64-bit value from \a element and converts it to a base value.
	template<
			typename TBaseValue,
			typename TBsonElement,
			typename X = std::enable_if_t<std::is_same_v<uint64_t, typename TBaseValue::ValueType>>>
	TBaseValue GetValue64(TBsonElement element) {
		return TBaseValue(static_cast<typename TBaseValue::ValueType>(element.get_int64().value));
	}

	/// Populates \a dest with data from \a source.
	template<typename TTag, typename TMongoContainer>
	void DbBinaryToModelArray(utils::ByteArray<TTag>& dest, const TMongoContainer& source) {
		if (dest.size() != source.size)
			CATAPULT_THROW_INVALID_ARGUMENT_1("invalid size of dest array", dest.size());

		std::memcpy(dest.data(), source.bytes, dest.size());
	}

	// endregion

	// region document helpers

	/// Attempts to read a uint64 value from \a documentView with \a name, returning \a defaultValue if no such value exists.
	uint64_t GetUint64OrDefault(const bsoncxx::document::view& documentView, const char* name, uint64_t defaultValue);

	/// Returns \c true if \a document is empty.
	bool IsEmptyDocument(const bsoncxx::document::value& document);

	// endregion

	// region streaming helpers

	/// Streams an embedded \a transaction to \a builder.
	bson_stream::document& StreamEmbeddedTransaction(bson_stream::document& builder, const model::EmbeddedTransaction& transaction);

	/// Streams a verifiable \a entity to \a builder.
	bson_stream::document& StreamVerifiableEntity(bson_stream::document& builder, const model::VerifiableEntity& entity);

	/// Streams a mosaic composed of \a id and \a amount to \a context.
	bson_stream::array_context& StreamMosaic(bson_stream::array_context& context, MosaicId id, Amount amount);

	/// Streams a mosaic composed of \a id and \a amount to \a context.
	bson_stream::array_context& StreamMosaic(bson_stream::array_context& context, UnresolvedMosaicId id, Amount amount);

	/// Streams \a receipt to \a builder.
	bson_stream::document& StreamReceipt(bson_stream::document& builder, const model::Receipt& receipt);

	// endregion
}}}
