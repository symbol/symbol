#pragma once
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <mongocxx/client.hpp>
#include <type_traits>

namespace catapult {
	namespace model {
		struct EmbeddedEntity;
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

	/// Converts raw array (\a pData) of \a size elements into bson binary type.
	bsoncxx::v_noabi::types::b_binary ToBinary(const uint8_t* pData, size_t size);

	/// Converts \a array into bson binary type.
	template<size_t N>
	auto ToBinary(const std::array<uint8_t, N>& array) {
		return ToBinary(array.data(), array.size());
	}

	/// Converts base \a value to int64_t.
	template<
		typename TBaseValue,
		typename X = typename std::enable_if<std::is_same<uint64_t, typename TBaseValue::ValueType>::value>::type>
	int64_t ToInt64(TBaseValue value) {
		return static_cast<int64_t>(value.unwrap());
	}

	/// Converts base \a value to int32_t.
	template<
		typename TBaseValue,
		typename X = typename std::enable_if<std::is_same<uint32_t, typename TBaseValue::ValueType>::value>::type>
	int32_t ToInt32(TBaseValue value) {
		return static_cast<int32_t>(value.unwrap());
	}

	/// Converts a 32 bit signed \a value to a 32 bit unsigned value.
	uint32_t ToUint32(int32_t value);

	/// Gets a 64-bit value from \a element and converts it to a base value.
	template<
			typename TBaseValue,
			typename TBsonElement,
			typename X = typename std::enable_if<std::is_same<uint64_t, typename TBaseValue::ValueType>::value>::type>
	TBaseValue GetValue64(TBsonElement element) {
		return TBaseValue(static_cast<typename TBaseValue::ValueType>(element.get_int64().value));
	}

	/// Populates \a dest with data from \a source.
	template<size_t N, typename TMongoContainer>
	void DbBinaryToModelArray(std::array<uint8_t, N>& dest, const TMongoContainer& source) {
		if (dest.size() != source.size)
			CATAPULT_THROW_INVALID_ARGUMENT_1("invalid size of dest array", dest.size());

		std::memcpy(dest.data(), source.bytes, dest.size());
	}

	/// Streams an embedded \a entity to \a builder.
	bson_stream::document& StreamEmbeddedEntity(bson_stream::document& builder, const model::EmbeddedEntity& entity);

	/// Streams a verifiable \a entity to \a builder.
	bson_stream::document& StreamVerifiableEntity(bson_stream::document& builder, const model::VerifiableEntity& entity);

	/// Streams a mosaic composed of \a id and \a amount to \a context.
	bson_stream::array_context& StreamMosaic(bson_stream::array_context& context, MosaicId id, Amount amount);
}}}
