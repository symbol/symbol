#pragma once
#include "ContainerDeltaPair.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/BaseSetDefaultTraits.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "tests/TestHarness.h"
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace deltaset {

	/// Test entity used as the base of both immutable and mutable test entities.
	struct TestEntity {
	public:
		TestEntity() : TestEntity("TestEntity", 0)
		{}

		explicit TestEntity(const std::string& name) : TestEntity(name, 0)
		{}

		explicit TestEntity(unsigned int value) : TestEntity("TestEntity", value)
		{}

		explicit TestEntity(const std::string& name, unsigned int value)
				: Name(name)
				, Value(value)
				, Hash(calculateHash())
				, Dummy(0)
				, HasherCallCount(0)
		{}

	public:
		bool operator<(const TestEntity& rhs) const {
			return Name < rhs.Name || (Name == rhs.Name && Value < rhs.Value);
		}

		bool operator==(const TestEntity& rhs) const {
			return Name == rhs.Name && Value == rhs.Value;
		}

	public:
		friend std::ostream& operator<<(std::ostream& out, const TestEntity& entity) {
			out << "TestEntity(" << entity.Name << ", " << entity.Value << ")" << std::endl;
			return out;
		}

	public:
		size_t calculateHash() {
			auto raw = Name + std::to_string(Value);
			auto checksum = 0u;
			for (auto ch : raw)
				checksum += static_cast<size_t>(ch);

			return checksum;
		}

	public:
		std::string Name;
		unsigned int Value;
		size_t Hash;
		mutable size_t Dummy;
		mutable size_t HasherCallCount;
	};

	/// Immutable test entity (deltas should not copy on write).
	struct ImmutableTestEntity : public TestEntity {
		using TestEntity::TestEntity;
	};

	/// Mutable test entity (deltas should copy on write).
	struct MutableTestEntity : public TestEntity {
		using TestEntity::TestEntity;

		void mutate()
		{}
	};

	// region Hasher

	/// Hashing function for TestEntity.
	template<typename T>
	struct Hasher {
		size_t operator()(const T& entity) const {
			++entity.HasherCallCount;
			return entity.Hash;
		}
	};

	template<typename T>
	struct Hasher<std::shared_ptr<T>> {
		size_t operator()(const std::shared_ptr<T>& pEntity) const {
			Hasher<T> hasher;
			return hasher(*pEntity);
		}
	};

	// endregion

	// region ReverseComparator

	/// Reverse comparator that reverses the natural sorting order of entities.
	template<typename T>
	struct ReverseComparator {
		bool operator()(const T& lhs, const T& rhs) const {
			return rhs < lhs;
		}
	};

	template<typename T>
	struct ReverseComparator<std::shared_ptr<T>> {
		bool operator()(const std::shared_ptr<T>& pLhs, const std::shared_ptr<T>& pRhs) const {
			ReverseComparator<T> comparator;
			return comparator(*pLhs, *pRhs);
		}
	};

	// endregion

	// region IsMutable

	/// Returns \c true if \a TEntity is mutable.
	template<typename TEntity>
	constexpr bool IsMutable() { return false; }

	template<>
	constexpr bool IsMutable<MutableTestEntity>() { return true; }

	template<>
	constexpr bool IsMutable<std::shared_ptr<MutableTestEntity>>() { return true; }

	// endregion

	// region EntityFactory

	/// Factory for creating and converting entities
	template<typename T>
	struct EntityFactory {
		using EntityType = T;

		static T CreateEntity(const std::string& name, unsigned int value) {
			return T(name, value);
		}

		template<typename TEntity>
		static TEntity* ToPointer(TEntity& entity) {
			return &entity;
		}

		template<typename TEntity>
		static TEntity& ToSetEntity(TEntity* pEntity) {
			return *pEntity;
		}
	};

	template<typename T>
	struct EntityFactory<std::shared_ptr<T>> {
		using EntityType = std::shared_ptr<T>;

		static std::shared_ptr<T> CreateEntity(const std::string& name, unsigned int value) {
			return std::make_shared<T>(name, value);
		}

		static std::shared_ptr<T> ToPointer(const std::shared_ptr<T>& pEntity) {
			return pEntity;
		}

		static std::shared_ptr<T> ToSetEntity(const std::shared_ptr<T>& pEntity) {
			return pEntity;
		}
	};

	/// Extends EntityFactory by adding batch functions.
	template<typename TBaseSetTraits>
	struct BatchEntityFactory : public EntityFactory<typename TBaseSetTraits::EntityType> {
		using Base = EntityFactory<typename TBaseSetTraits::EntityType>;
		using EntityType = typename Base::EntityType;
		using EntityVector = std::vector<typename std::remove_const<EntityType>::type>;

		static EntityVector CreateEntities(size_t count) {
			EntityVector entities;
			for (auto i = 0u; i < count; ++i)
				entities.push_back(Base::CreateEntity("TestEntity", i));

			return entities;
		}

		template<typename TBaseSetDelta>
		static void InsertEntities(TBaseSetDelta& delta, size_t count) {
			for (auto i = 0u; i < count; ++i)
				delta.insert(Base::CreateEntity("TestEntity", i));
		}

		template<typename TSet>
		static void AssertContents(const TSet& set, const EntityVector& expectedEntities) {
			EXPECT_EQ(expectedEntities.size(), set.size());
			for (const auto& entity : expectedEntities) {
				auto pEntityFromSet = set.find(ToKey(entity));
				EXPECT_EQ(*Base::ToPointer(entity), *pEntityFromSet);
			}
		}

		static auto ToKey(const EntityType& entity) {
			return TBaseSetTraits::SetTraits::ToKey(entity);
		}

		static auto CreateKey(const std::string& name, unsigned int value) {
			return ToKey(Base::CreateEntity(name, value));
		}

		static auto ToPointerFromStorage(const typename TBaseSetTraits::SetTraits::StorageType& storage) {
			return Base::ToPointer(TBaseSetTraits::SetTraits::ToValue(storage));
		}

		static bool IsEntityMutable() {
			return IsMutable<EntityType>();
		}
	};

	// endregion

	/// Traits for creating base sets.
	template<typename TBaseSetTraits>
	struct BaseTraits : BatchEntityFactory<TBaseSetTraits> {

		static auto Create() {
			return TBaseSetTraits::Create();
		}

		static auto CreateWithEntities(size_t count) {
			auto pBaseSet = Create();
			auto pDelta = pBaseSet->rebase();
			BatchEntityFactory<TBaseSetTraits>::InsertEntities(*pDelta, count);
			TBaseSetTraits::Commit(*pBaseSet);
			return pBaseSet;
		}
	};

	/// Traits for creating deltas.
	template<typename TBaseSetTraits>
	struct DeltaTraits : BatchEntityFactory<TBaseSetTraits> {

		static auto Create() {
			auto pBaseSet = CreateBase();
			return test::ContainerDeltaPair<TBaseSetTraits>(pBaseSet, pBaseSet->rebase());
		}

		static auto CreateWithEntities(size_t count) {
			auto pBaseSet = CreateBase();
			auto pDelta = pBaseSet->rebase();
			BatchEntityFactory<TBaseSetTraits>::InsertEntities(*pDelta, count);
			return test::ContainerDeltaPair<TBaseSetTraits>(pBaseSet, pDelta);
		}

		static auto CreateBase() {
			return TBaseSetTraits::Create();
		}
	};

	template<typename TEntity>
	using OrderedSetTraits = SetStorageTraits<std::set<TEntity, DefaultComparator<TEntity>>>;

	template<typename TEntity>
	using UnorderedSetTraits = SetStorageTraits<std::unordered_set<TEntity, Hasher<TEntity>, EqualityChecker<TEntity>>>;

	template<typename TEntity>
	using ReverseOrderedSetTraits = SetStorageTraits<std::set<TEntity, ReverseComparator<TEntity>>>;

	struct MapKeyHasher {
		size_t operator()(const std::pair<std::string, unsigned int>& pair) const {
			std::hash<std::string> stringHasher;
			std::hash<unsigned int> intHasher;
			return stringHasher(pair.first) ^ intHasher(pair.second);
		}
	};

	template<typename T>
	struct TestEntityToKeyConverter {
		static auto ToKey(const T& entity) {
			const auto& entityRef = detail::DerefHelper<T>::Deref(entity);
			return std::make_pair(entityRef.Name, entityRef.Value);
		}
	};

	template<typename T>
	using UnorderedMapSetTraits = MapStorageTraits<
			std::unordered_map<std::pair<std::string, unsigned int>, T, MapKeyHasher>,
			TestEntityToKeyConverter<T>>;

	template<typename TMutabilityTraits>
	using SetEntityType = typename std::remove_const<typename TMutabilityTraits::EntityType>::type;

	template<typename TEntityTraits, typename TSetTraits>
	struct BaseSetTraits {
		using Type = BaseSet<TEntityTraits, TSetTraits>;
		using DeltaType = BaseSetDelta<TEntityTraits, TSetTraits>;
		using EntityType = typename TEntityTraits::EntityType;
		using SetTraits = TSetTraits;

		static auto Create() {
			return CreateBaseSet<TEntityTraits, TSetTraits>();
		}

		static void Commit(Type& set) {
			set.commit();
		}
	};

	template<typename TMutabilityTraits>
	using OrderedTraits = BaseSetTraits<
			TMutabilityTraits,
			OrderedSetTraits<SetEntityType<TMutabilityTraits>>>;

	template<typename TMutabilityTraits>
	using UnorderedTraits = BaseSetTraits<
			TMutabilityTraits,
			UnorderedSetTraits<SetEntityType<TMutabilityTraits>>>;

	template<typename TMutabilityTraits>
	using ReverseOrderedTraits = BaseSetTraits<
			TMutabilityTraits,
			ReverseOrderedSetTraits<SetEntityType<TMutabilityTraits>>>;

	template<typename TMutabilityTraits>
	using UnorderedMapTraits = BaseSetTraits<
			TMutabilityTraits,
			UnorderedMapSetTraits<SetEntityType<TMutabilityTraits>>>;
}}
