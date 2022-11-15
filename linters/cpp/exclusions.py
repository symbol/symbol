import re

# those files won't be checked at all
SKIP_FILES = (
	# macro-based enums
	re.compile(r'src.catapult.utils.MacroBasedEnum.h'),

	# inline includes
	re.compile(r'src.catapult.model.EntityType.cpp'),
	re.compile(r'src.catapult.model.ReceiptType.cpp'),
	re.compile(r'src.catapult.validators.ValidationResult.cpp'),
	re.compile(r'tools.statusgen.main.cpp')
)

NAMESPACES_FALSEPOSITIVES = (
	# multiple namespaces (specialization)
	re.compile(r'src.catapult.utils.Logging.cpp'),  # (boost::log)
	re.compile(r'tests.catapult.deltaset.ConditionalContainerTests.cpp'),  # (catapult::test)
	re.compile(r'tests.TestHarness.h'),  # (std)
	re.compile(r'tools.health.ApiNodeHealthUtils.cpp'),  # (boost::asio)

	# disallowed top-level namespaces
	re.compile(r'src.catapult.thread.detail.FutureSharedState.h'),  # (detail)

	# no types (only includes and/or fwd declares and/or defines)
	re.compile(r'src.catapult.constants.h'),
	re.compile(r'src.catapult.plugins.h'),
	re.compile(r'src.catapult.preprocessor.h'),
	re.compile(r'src.catapult.cache_db.RocksInclude.h'),
	re.compile(r'src.catapult.utils.BitwiseEnum.h'),
	re.compile(r'src.catapult.utils.ExceptionLogging.h'),
	re.compile(r'src.catapult.utils.MacroBasedEnumIncludes.h'),
	re.compile(r'src.catapult.version.version_inc.h'),
	re.compile(r'src.catapult.version.nix.what_version.cpp'),

	re.compile(r'extensions.mongo.src.mappers.MapperInclude.h'),
	re.compile(r'extensions.mongo.src.CacheStorageInclude.h'),
	re.compile(r'plugins.txes.lock_shared.src.validators.LockDurationValidator.h'),
	re.compile(r'plugins.txes.mosaic.src.model.MosaicConstants.h'),
	re.compile(r'plugins.txes.namespace.src.model.NamespaceConstants.h'),
	re.compile(r'tests.test.nodeps.Stress.h'),
	re.compile(r'internal.tools.*Generators.h'),

	# cache aliases (only headers without 'real' types)
	re.compile(r'plugins.services.hashcache.src.cache.HashCacheTypes.h'),
	re.compile(r'plugins.txes.mosaic.src.cache.MosaicCacheTypes.h'),
	re.compile(r'plugins.txes.multisig.src.cache.MultisigCacheTypes.h'),
	re.compile(r'plugins.txes.namespace.src.cache.NamespaceCacheTypes.h'),

	# main entry points
	re.compile(r'src.catapult.process.broker.main.cpp'),
	re.compile(r'src.catapult.process.importer.main.cpp'),
	re.compile(r'src.catapult.process.recovery.main.cpp'),
	re.compile(r'src.catapult.process.server.main.cpp'),
	re.compile(r'tests.bench.nodeps.BenchMain.cpp'),

	# mongo plugins (only entry point)
	re.compile(r'extensions.mongo.plugins.*.src.Mongo.*Plugin.cpp')
)

EMPTYLINES_FALSEPOSITIVES = (
)

LONGLINES_FALSEPOSITIVES = (
)

SPECIAL_INCLUDES = (
	# src (these include double quotes because they have to match what is after `#include `)
	re.compile(r'"catapult/utils/MacroBasedEnum\.h"'),
	re.compile(r'"ReentrancyCheckReaderNotificationPolicy.h"'),

	re.compile(r'<ref10/crypto_verify_32.h>'),

	# those always should be in an ifdef
	re.compile(r'<dlfcn.h>'),
	re.compile(r'<io.h>'),
	re.compile(r'<mach/mach.h>'),
	re.compile(r'<psapi.h>'),
	re.compile(r'<stdexcept>'),
	re.compile(r'<sys/file.h>'),
	re.compile(r'<sys/resource.h>'),
	re.compile(r'<sys/time.h>'),
	re.compile(r'<unistd.h>'),
	re.compile(r'<windows.h>')
)

CORE_FIRSTINCLUDES = {
	# src
	'src/catapult/consumers/BatchSignatureConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/BlockchainCheckConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/BlockchainSyncCleanupConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/BlockchainSyncConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/HashCalculatorConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/HashCheckConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/NewBlockConsumer.cpp': 'BlockConsumers.h',
	'src/catapult/consumers/NewTransactionsConsumer.cpp': 'ConsumerResultFactory.h',
	'src/catapult/consumers/StatelessValidationConsumer.cpp': 'BlockConsumers.h',

	'src/catapult/ionet/IoEnums.cpp': 'ConnectResult.h',
	'src/catapult/net/NetEnums.cpp': 'NodeRequestResult.h',
	'src/catapult/process/broker/main.cpp': 'catapult/extensions/ProcessBootstrapper.h',
	'src/catapult/process/importer/main.cpp': 'catapult/extensions/ProcessBootstrapper.h',
	'src/catapult/process/recovery/main.cpp': 'catapult/extensions/ProcessBootstrapper.h',
	'src/catapult/process/server/main.cpp': 'catapult/extensions/ProcessBootstrapper.h',
	'src/catapult/version/nix/what_version.cpp': 'catapult/version/version.h',

	# tests
	'tests/test/nodeps/TestMain.cpp': 'catapult/crypto/OpensslInit.h',

	'tests/catapult/consumers/BatchSignatureConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
	'tests/catapult/consumers/BlockchainCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
	'tests/catapult/consumers/BlockchainSyncCleanupConsumerTests.cpp': 'catapult/config/CatapultDataDirectory.h',
	'tests/catapult/consumers/BlockchainSyncConsumerTests.cpp': 'catapult/cache_core/AccountStateCache.h',
	'tests/catapult/consumers/HashCalculatorConsumerTests.cpp': 'sdk/src/extensions/BlockExtensions.h',
	'tests/catapult/consumers/HashCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
	'tests/catapult/consumers/NewBlockConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
	'tests/catapult/consumers/NewTransactionsConsumerTests.cpp': 'catapult/consumers/TransactionConsumers.h',
	'tests/catapult/consumers/StatelessValidationConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',

	'tests/catapult/deltaset/MapVirtualizedTests.cpp': 'tests/catapult/deltaset/test/BaseSetDeltaTests.h',
	'tests/catapult/deltaset/OrderedTests.cpp': 'tests/catapult/deltaset/test/BaseSetDeltaTests.h',
	'tests/catapult/deltaset/ReverseOrderedTests.cpp': 'tests/catapult/deltaset/test/BaseSetDeltaTests.h',
	'tests/catapult/deltaset/SetVirtualizedTests.cpp': 'tests/catapult/deltaset/test/BaseSetDeltaTests.h',
	'tests/catapult/deltaset/UnorderedMapTests.cpp': 'tests/catapult/deltaset/test/BaseSetDeltaTests.h',
	'tests/catapult/deltaset/UnorderedTests.cpp': 'tests/catapult/deltaset/test/BaseSetDeltaTests.h',
	'tests/catapult/thread/FutureSharedStateTests.cpp': 'catapult/thread/detail/FutureSharedState.h',
	'tests/catapult/utils/CatapultExceptionTests.cpp': 'catapult/exceptions.h',
	'tests/catapult/utils/CatapultTypesTests.cpp': 'catapult/types.h',
	'tests/catapult/utils/CountOfTests.cpp': 'catapult/types.h',
	'tests/catapult/utils/MacroBasedEnumTests.cpp': 'catapult/utils/MacroBasedEnumIncludes.h',
	'tests/catapult/utils/TraitsTests.cpp': 'catapult/utils/traits/Traits.h',
	'tests/catapult/utils/StlTraitsTests.cpp': 'catapult/utils/traits/StlTraits.h',

	'tests/int/node/basic/LocalNodeBasicTests.cpp': 'tests/int/node/test/LocalNodeBasicTests.h',
	'tests/int/stress/NemesisBlockLoaderIntegrityTests.cpp': 'catapult/extensions/NemesisBlockLoader.h',
	'tests/int/stress/UtUpdaterIntegrityTests.cpp': 'catapult/chain/UtUpdater.h'
}

PLUGINS_FIRSTINCLUDES = {
	# plugins
	'plugins/txes/mosaic/src/validators/ActiveMosaicView.cpp': 'ActiveMosaicView.h',
	'plugins/txes/restriction_account/src/validators/AccountRestrictionView.cpp': 'AccountRestrictionView.h',

	'plugins/coresystem/src/validators/KeyLinkActionValidator.cpp': 'KeyLinkValidators.h',
	'plugins/coresystem/src/validators/VotingKeyLinkRangeValidator.cpp': 'KeyLinkValidators.h',
	'plugins/coresystem/tests/importance/PosImportanceCalculatorTests.cpp': 'src/importance/ImportanceCalculator.h',
	'plugins/coresystem/tests/importance/RestoreImportanceCalculatorTests.cpp': 'src/importance/ImportanceCalculator.h',
	'plugins/coresystem/tests/observers/RecalculateImportancesObserverTests.cpp': 'src/importance/ImportanceCalculator.h',
	'plugins/coresystem/tests/validators/KeyLinkActionValidatorTests.cpp': 'src/validators/KeyLinkValidators.h',
	'plugins/coresystem/tests/validators/VotingKeyLinkRangeValidatorTests.cpp': 'src/validators/KeyLinkValidators.h',

	'plugins/coresystem/tests/validators/ZeroAccountValidatorTests.cpp': 'sdk/src/extensions/ConversionExtensions.h',

	'plugins/services/hashcache/tests/observers/TransactionHashObserverTests.cpp': 'src/cache/HashCache.h',
	'plugins/services/hashcache/tests/validators/UniqueTransactionHashValidatorTests.cpp': 'src/cache/HashCache.h',

	'plugins/txes/account_link/tests/validators/RemoteInteractionValidatorTests.cpp': 'src/model/AccountKeyLinkTransaction.h',
	'plugins/txes/aggregate/tests/validators/AggregateTransactionVersionValidatorTests.cpp': 'src/model/AggregateEntityType.h',
	'plugins/txes/lock_hash/tests/observers/CompletedAggregateObserverTests.cpp': 'src/model/HashLockReceiptType.h',
	'plugins/txes/lock_hash/tests/observers/ExpiredHashLockInfoObserverTests.cpp': 'src/model/HashLockReceiptType.h',
	'plugins/txes/lock_hash/tests/observers/HashLockObserverTests.cpp': 'src/model/HashLockReceiptType.h',
	'plugins/txes/lock_hash/tests/validators/HashLockCacheUniqueValidatorTests.cpp': 'plugins/txes/lock_shared/tests/validators/LockCacheUniqueValidatorTests.h',
	'plugins/txes/lock_hash/tests/validators/HashLockDurationValidatorTests.cpp': 'src/state/HashLockInfo.h',

	'plugins/txes/lock_secret/tests/observers/ExpiredSecretLockInfoObserverTests.cpp': 'src/model/SecretLockReceiptType.h',
	'plugins/txes/lock_secret/tests/observers/ProofObserverTests.cpp': 'src/model/SecretLockReceiptType.h',
	'plugins/txes/lock_secret/tests/observers/SecretLockObserverTests.cpp': 'src/model/SecretLockReceiptType.h',
	'plugins/txes/lock_secret/tests/validators/ProofSecretValidatorTests.cpp': 'src/model/LockHashUtils.h',
	'plugins/txes/lock_secret/tests/validators/SecretLockCacheUniqueValidatorTests.cpp': 'plugins/txes/lock_shared/tests/validators/LockCacheUniqueValidatorTests.h',
	'plugins/txes/lock_secret/tests/validators/SecretLockDurationValidatorTests.cpp': 'src/state/SecretLockInfo.h',

	'plugins/txes/metadata/tests/model/MetadataTransactionTests.cpp': 'src/model/AccountMetadataTransaction.h',
	'plugins/txes/mosaic/tests/observers/MosaicDefinitionObserverTests.cpp': 'src/cache/MosaicCache.h',
	'plugins/txes/mosaic/tests/validators/ActiveMosaicViewTests.cpp': 'src/validators/ActiveMosaicView.h',
	'plugins/txes/mosaic/tests/validators/MosaicAvailabilityValidatorTests.cpp': 'src/cache/MosaicCache.h',
	'plugins/txes/mosaic/tests/validators/MosaicDurationValidatorTests.cpp': 'src/cache/MosaicCache.h',
	'plugins/txes/mosaic/tests/validators/MosaicIdValidatorTests.cpp': 'src/model/MosaicIdGenerator.h',
	'plugins/txes/multisig/tests/validators/MultisigAggregateEligibleCosignatoriesValidatorTests.cpp': 'src/plugins/MultisigAccountModificationTransactionPlugin.h',
	'plugins/txes/multisig/tests/validators/MultisigAggregateSufficientCosignatoriesValidatorTests.cpp': 'src/plugins/MultisigAccountModificationTransactionPlugin.h',
	'plugins/txes/multisig/tests/validators/MultisigInvalidCosignatoriesValidatorTests.cpp': 'src/cache/MultisigCache.h',
	'plugins/txes/multisig/tests/validators/MultisigInvalidSettingsValidatorTests.cpp': 'src/cache/MultisigCache.h',
	'plugins/txes/namespace/tests/validators/NamespaceAvailabilityValidatorTests.cpp': 'src/cache/NamespaceCache.h',
	'plugins/txes/namespace/tests/validators/NamespaceDurationOverflowValidatorTests.cpp': 'src/model/NamespaceConstants.h',
	'plugins/txes/namespace/tests/validators/NamespaceNameValidatorTests.cpp': 'src/model/NamespaceIdGenerator.h',
	'plugins/txes/namespace/tests/validators/RootNamespaceMaxChildrenValidatorTests.cpp': 'src/cache/NamespaceCache.h',
	'plugins/txes/restriction_account/tests/model/AccountRestrictionTransactionTests.cpp': 'src/model/AccountAddressRestrictionTransaction.h',
	'plugins/txes/restriction_account/tests/validators/AccountOperationRestrictionNoSelfBlockingValidatorTests.cpp': 'src/model/AccountOperationRestrictionTransaction.h',
	'plugins/txes/restriction_account/tests/validators/AccountRestrictionValueModificationValidatorTests.cpp': 'sdk/src/extensions/ConversionExtensions.h',
	'plugins/txes/restriction_account/tests/validators/AccountRestrictionViewTests.cpp': 'src/validators/AccountRestrictionView.h',
	'plugins/txes/restriction_account/tests/validators/MaxAccountRestrictionValuesValidatorTests.cpp': 'sdk/src/extensions/ConversionExtensions.h',

	# sdk
	'sdk/tests/builders/AliasBuilderTests.cpp': 'src/builders/AddressAliasBuilder.h',
	'sdk/tests/builders/AccountRestrictionBuilderTests.cpp': 'src/builders/AccountAddressRestrictionBuilder.h',
	'sdk/tests/builders/KeyLinkBuilderTests.cpp': 'src/builders/AccountKeyLinkBuilder.h',
	'sdk/tests/builders/MetadataBuilderTests.cpp': 'src/builders/AccountMetadataBuilder.h'
}

TOOLS_FIRSTINCLUDES = {
}

EXTENSION_FIRSTINCLUDES = {
	'extensions/mongo/plugins/metadata/src/MongoMetadataPlugin.cpp': 'AccountMetadataMapper.h',
	'extensions/mongo/plugins/mosaic/src/MongoMosaicPlugin.cpp': 'MosaicDefinitionMapper.h',
	'extensions/mongo/plugins/multisig/src/MongoMultisigPlugin.cpp': 'MultisigAccountModificationMapper.h',
	'extensions/mongo/plugins/namespace/src/MongoNamespacePlugin.cpp': 'AddressAliasMapper.h',
	'extensions/mongo/plugins/restriction_mosaic/src/MongoMosaicRestrictionPlugin.cpp': 'MosaicAddressRestrictionMapper.h',

	'extensions/timesync/src/filters/ClampingFilter.cpp': 'SynchronizationFilters.h',
	'extensions/timesync/src/filters/ResponseDelayDetectionFilter.cpp': 'SynchronizationFilters.h',
	'extensions/timesync/src/filters/ReversedTimestampsFilter.cpp': 'SynchronizationFilters.h',

	'extensions/timesync/tests/filters/ClampingFilterTests.cpp': 'timesync/src/filters/SynchronizationFilters.h',
	'extensions/timesync/tests/filters/ResponseDelayDetectionFilterTests.cpp': 'timesync/src/filters/SynchronizationFilters.h',
	'extensions/timesync/tests/filters/ReversedTimestampsFilterTests.cpp': 'timesync/src/filters/SynchronizationFilters.h'
}

SKIP_FORWARDS = (
	re.compile(r'src.catapult.validators.ValidatorTypes.h'),
	re.compile(r'src.catapult.utils.ClampedBaseValue.h'),
	re.compile(r'.*\.cpp$')
)

FILTER_NAMESPACES = (
	re.compile(r'.*detail'),
	re.compile(r'.*_types::'),
	re.compile(r'.*_types$'),
	re.compile(r'.*bson_stream$')
)
