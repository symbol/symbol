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
#include "src/cache/PropertyCache.h"
#include "src/cache/PropertyCacheStorage.h"
#include "src/state/PropertyUtils.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/EntityType.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/nodeps/Conversions.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	/// Cache factory for creating a catapult cache composed of property cache and core caches.
	struct PropertyCacheFactory {
	private:
		static auto CreateSubCachesWithPropertyCache() {
			auto cacheId = cache::PropertyCache::Id;
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(cacheId + 1);
			subCaches[cacheId] = MakeSubCachePlugin<cache::PropertyCache, cache::PropertyCacheStorage>(model::NetworkIdentifier::Zero);
			return subCaches;
		}

	public:
		/// Creates an empty catapult cache around default configuration.
		static cache::CatapultCache Create() {
			return Create(model::BlockChainConfiguration::Uninitialized());
		}

		/// Creates an empty catapult cache around \a config.
		static cache::CatapultCache Create(const model::BlockChainConfiguration& config) {
			auto subCaches = CreateSubCachesWithPropertyCache();
			CoreSystemCacheFactory::CreateSubCaches(config, subCaches);
			return cache::CatapultCache(std::move(subCaches));
		}
	};

	struct BaseAddressPropertyTraits {
		using UnresolvedValueType = UnresolvedAddress;
		using ValueType = Address;

		static constexpr auto PropertyType() {
			return model::PropertyType::Address;
		}

		static constexpr auto PropertyValueSize() {
			return Address_Decoded_Size;
		}

		static auto RandomUnresolvedValue() {
			return test::GenerateRandomUnresolvedAddress();
		}

		static auto RandomValue() {
			return test::GenerateRandomData<Address_Decoded_Size>();
		}

		static UnresolvedValueType Unresolve(const ValueType& value) {
			return test::UnresolveXor(value);
		}

		static auto FromBuffer(const RawBuffer& buffer) {
			UnresolvedValueType address;
			std::memcpy(address.data(), buffer.pData, buffer.Size);
			return address;
		}
	};

	struct BaseMosaicPropertyTraits {
		using UnresolvedValueType = UnresolvedMosaicId;
		using ValueType = MosaicId;

		static constexpr auto PropertyType() {
			return model::PropertyType::MosaicId;
		}

		static constexpr auto PropertyValueSize() {
			return sizeof(ValueType);
		}

		static auto RandomUnresolvedValue() {
			return test::GenerateRandomValue<UnresolvedValueType>();
		}

		static auto RandomValue() {
			return test::GenerateRandomValue<ValueType>();
		}

		static UnresolvedValueType Unresolve(const ValueType& value) {
			return test::UnresolveXor(value);
		}

		static auto FromBuffer(const RawBuffer& buffer) {
			return reinterpret_cast<const UnresolvedValueType&>(*buffer.pData);
		}
	};

	struct BaseTransactionTypePropertyTraits {
		using UnresolvedValueType = model::EntityType;
		using ValueType = model::EntityType;

		static constexpr auto PropertyType() {
			return model::PropertyType::TransactionType;
		}

		static constexpr auto PropertyValueSize() {
			return sizeof(ValueType);
		}

		static auto RandomUnresolvedValue() {
			return static_cast<UnresolvedValueType>(test::RandomByte());
		}

		static auto RandomValue() {
			return static_cast<ValueType>(test::RandomByte());
		}

		static UnresolvedValueType Unresolve(const ValueType& value) {
			return value;
		}

		static auto FromBuffer(const RawBuffer& buffer) {
			return reinterpret_cast<const UnresolvedValueType&>(*buffer.pData);
		}
	};

	/// Traits for operation type 'Allow'.
	struct AllowTraits {
		/// Given \a propertyType gets the property type including the operation type.
		static model::PropertyType CompletePropertyType(model::PropertyType propertyType) {
			return propertyType;
		}

		/// Given \a propertyType gets the property type including the opposite operation type.
		static model::PropertyType OppositeCompletePropertyType(model::PropertyType propertyType) {
			return propertyType | model::PropertyType::Block;
		}

		/// Adds \a value to \a property for operation type 'Allow'.
		static void Add(state::AccountProperty& property, const state::AccountProperty::RawPropertyValue& value) {
			property.allow({ model::PropertyModificationType::Add, value });
		}
	};

	/// Traits for operation type 'Block'.
	struct BlockTraits {
		/// Given \a propertyType gets the property type including the operation type.
		static model::PropertyType CompletePropertyType(model::PropertyType propertyType) {
			return propertyType | model::PropertyType::Block;
		}

		/// Given \a propertyType gets the property type including the opposite operation type.
		static model::PropertyType OppositeCompletePropertyType(model::PropertyType propertyType) {
			return propertyType;
		}

		/// Adds \a value to \a property for operation type 'Block'.
		static void Add(state::AccountProperty& property, const state::AccountProperty::RawPropertyValue& value) {
			property.block({ model::PropertyModificationType::Add, value });
		}
	};

	/// Creates a vector with \a count unique values.
	template<typename TPropertyValueTraits>
	auto CreateRandomUniqueValues(size_t count) {
		std::set<typename TPropertyValueTraits::ValueType> values;
		while (values.size() < count)
			values.insert(TPropertyValueTraits::RandomValue());

		return std::vector<typename TPropertyValueTraits::ValueType>(values.cbegin(), values.cend());
	}

	/// Creates a value that is not contained in \a values and transforms the value using \a transform.
	template<typename TPropertyValueTraits, typename TTransform>
	auto CreateDifferentValue(const std::vector<typename TPropertyValueTraits::ValueType>& values, TTransform transform) {
		while (true) {
			auto randomValue = TPropertyValueTraits::RandomValue();
			auto isFound = std::any_of(values.cbegin(), values.cend(), [&randomValue](const auto& value) {
				return value == randomValue;
			});
			if (!isFound)
				return transform(randomValue);
		}
	}

	/// Populates \a delta with \a key and \a values.
	template<typename TPropertyValueTraits, typename TOperationTraits = AllowTraits>
	void PopulateCache(
			cache::CatapultCacheDelta& delta,
			const Key& key,
			const std::vector<typename TPropertyValueTraits::ValueType>& values) {
		auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
		auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
		propertyCacheDelta.insert(state::AccountProperties(address));
		auto& accountProperties = propertyCacheDelta.find(address).get();
		auto& accountProperty = accountProperties.property(TPropertyValueTraits::PropertyType());
		for (const auto& value : values)
			TOperationTraits::Add(accountProperty, state::ToVector(value));
	}

	/// Populates \a cache with \a key and \a values.
	template<typename TPropertyValueTraits, typename TOperationTraits = AllowTraits>
	void PopulateCache(cache::CatapultCache& cache, const Key& key, const std::vector<typename TPropertyValueTraits::ValueType>& values) {
		auto delta = cache.createDelta();
		PopulateCache<TPropertyValueTraits, TOperationTraits>(delta, key, values);
		cache.commit(Height(1));
	}

	/// Creates a notification around \a key and \a modification.
	template<typename TPropertyValueTraits, typename TOperationTraits = AllowTraits>
	auto CreateNotification(
			const Key& key,
			const model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>& modification) {
		return typename TPropertyValueTraits::NotificationType{
			key,
			TOperationTraits::CompletePropertyType(TPropertyValueTraits::PropertyType()),
			modification
		};
	}

	/// Creates a notification with opposite operation type around \a key and \a modification.
	template<typename TPropertyValueTraits, typename TOperationTraits = AllowTraits>
	auto CreateNotificationWithOppositeOperation(
			const Key& key,
			const model::PropertyModification<typename TPropertyValueTraits::UnresolvedValueType>& modification) {
		return typename TPropertyValueTraits::NotificationType{
			key,
			TOperationTraits::OppositeCompletePropertyType(TPropertyValueTraits::PropertyType()),
			modification
		};
	}

	/// Creates a notification around \a key and \a modifications.
	template<typename TPropertyValueTraits, typename TValueType, typename TOperationTraits = AllowTraits>
	auto CreateNotification(const Key& key, const std::vector<model::PropertyModification<TValueType>>& modifications) {
		return typename TPropertyValueTraits::NotificationType{
			key,
			TOperationTraits::CompletePropertyType(TPropertyValueTraits::PropertyType()),
			utils::checked_cast<size_t, uint8_t>(modifications.size()),
			modifications.data()
		};
	}
}}
