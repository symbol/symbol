#pragma once
#include "AccountStateCacheTypes.h"
#include "ReadOnlyAccountStateCache.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/model/ContainerTypes.h"

namespace catapult { namespace cache {

	/// Mixins used by the account state cache view.
	struct AccountStateCacheViewMixins {
		using KeyLookupAdapter = AccountStateCacheTypes::ComposedLookupAdapter<AccountStateCacheTypes::ComposableBaseSets>;

		using Size = SizeMixin<AccountStateCacheTypes::PrimaryTypes::BaseSetType>;
		using ContainsAddress = ContainsMixin<AccountStateCacheTypes::PrimaryTypes::BaseSetType, AccountStateCacheDescriptor>;
		using ContainsKey = ContainsMixin<
			AccountStateCacheTypes::KeyLookupMapTypes::BaseSetType,
			AccountStateCacheTypes::KeyLookupMapTypesDescriptor>;
		using MapIteration = MapIterationMixin<AccountStateCacheTypes::PrimaryTypes::BaseSetType, AccountStateCacheDescriptor>;
		using ConstAccessorAddress = ConstAccessorMixin<
			AccountStateCacheTypes::PrimaryTypes::BaseSetType,
			AccountStateCacheDescriptor,
			AccountStateCacheTypes::ConstValueAdapter>;
		using ConstAccessorKey = ConstAccessorMixin<KeyLookupAdapter, KeyLookupAdapter, AccountStateCacheTypes::ConstValueAdapter>;
	};

	/// Basic view on top of the account state cache.
	class BasicAccountStateCacheView
			: public utils::MoveOnly
			, public AccountStateCacheViewMixins::Size
			, public AccountStateCacheViewMixins::ContainsAddress
			, public AccountStateCacheViewMixins::ContainsKey
			, public AccountStateCacheViewMixins::MapIteration
			, public AccountStateCacheViewMixins::ConstAccessorAddress
			, public AccountStateCacheViewMixins::ConstAccessorKey {
	public:
		using ReadOnlyView = ReadOnlyAccountStateCache;

	public:
		/// Creates a view around \a accountStateSets, \a options and \a highValueAddresses.
		BasicAccountStateCacheView(
				const AccountStateCacheTypes::BaseSetType& accountStateSets,
				const AccountStateCacheTypes::Options& options,
				const model::AddressSet& highValueAddresses);

	private:
		BasicAccountStateCacheView(
				const AccountStateCacheTypes::BaseSetType& accountStateSets,
				const AccountStateCacheTypes::Options& options,
				const model::AddressSet& highValueAddresses,
				std::unique_ptr<AccountStateCacheViewMixins::KeyLookupAdapter>&& pKeyLookupAdapter);

	public:
		using AccountStateCacheViewMixins::ContainsAddress::contains;
		using AccountStateCacheViewMixins::ContainsKey::contains;

		using AccountStateCacheViewMixins::ConstAccessorAddress::get;
		using AccountStateCacheViewMixins::ConstAccessorKey::get;

		using AccountStateCacheViewMixins::ConstAccessorAddress::tryGet;
		using AccountStateCacheViewMixins::ConstAccessorKey::tryGet;

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const;

		/// Gets the network importance grouping.
		uint64_t importanceGrouping() const;

	private:
		const model::NetworkIdentifier m_networkIdentifier;
		const uint64_t m_importanceGrouping;
		std::unique_ptr<AccountStateCacheViewMixins::KeyLookupAdapter> m_pKeyLookupAdapter;
	};

	/// View on top of the account state cache.
	class AccountStateCacheView : public ReadOnlyViewSupplier<BasicAccountStateCacheView> {
	public:
		/// Creates a view around \a accountStateSets, \a options and \a highValueAddresses.
		AccountStateCacheView(
				const AccountStateCacheTypes::BaseSetType& accountStateSets,
				const AccountStateCacheTypes::Options& options,
				const model::AddressSet& highValueAddresses)
				: ReadOnlyViewSupplier(accountStateSets, options, highValueAddresses)
		{}
	};
}}
