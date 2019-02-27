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
    re.compile(r'src.catapult.cache_db.RocksInclude.h'),
    re.compile(r'src.catapult.crypto.KeccakHash.h'),
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
    re.compile(r'plugins.txes.property.src.model.PropertyNotifications.h'),
    re.compile(r'tests.test.nodeps.Stress.h'),
    re.compile(r'internal.tools.*Generators.h'),

    # cache aliases (only headers without 'real' types)
    re.compile(r'plugins.services.hashcache.src.cache.HashCacheTypes.h'),
    re.compile(r'plugins.txes.mosaic.src.cache.MosaicCacheTypes.h'),
    re.compile(r'plugins.txes.multisig.src.cache.MultisigCacheTypes.h'),
    re.compile(r'plugins.txes.namespace.src.cache.NamespaceCacheTypes.h'),

    # main entry points
    re.compile(r'src.catapult.server.main.cpp'),
    re.compile(r'tests.bench.nodeps.BenchMain.cpp'),

    # mongo plugins (only entry point)
    re.compile(r'extensions.mongo.plugins.*.src.Mongo.*Plugin.cpp'),
)

EMPTYLINES_FALSEPOSITIVES = (
)

LONGLINES_FALSEPOSITIVES = (
    # 64-byte hex strings
    re.compile(r'SignerTests.cpp'),
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
    re.compile(r'<windows.h>'),
)

CORE_FIRSTINCLUDES = {
    # src
    'src/catapult/consumers/AddressExtractionConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/BlockChainCheckConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/BlockChainSyncConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/HashCalculatorConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/HashCheckConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/NewBlockConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/NewTransactionsConsumer.cpp': 'TransactionConsumers.h',
    'src/catapult/consumers/StatelessValidationConsumer.cpp': 'BlockConsumers.h',

    'src/catapult/ionet/IoEnums.cpp': 'ConnectionSecurityMode.h',
    'src/catapult/net/NetEnums.cpp': 'NodeRequestResult.h',
    'src/catapult/server/main.cpp': 'ServerMain.h',
    'src/catapult/version/nix/what_version.cpp': 'catapult/version/version.h',

    # tests
    'tests/test/nodeps/TestMain.cpp': 'catapult/utils/ConfigurationValueParsers.h',

    'tests/catapult/consumers/AddressExtractionConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/BlockChainCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/BlockChainSyncConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/HashCalculatorConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
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
    'tests/catapult/io/MemoryStreamTests.cpp': 'tests/test/core/mocks/MockMemoryStream.h',
    'tests/catapult/thread/FutureSharedStateTests.cpp': 'catapult/thread/detail/FutureSharedState.h',
    'tests/catapult/utils/CatapultExceptionTests.cpp': 'catapult/exceptions.h',
    'tests/catapult/utils/CatapultTypesTests.cpp': 'catapult/types.h',
    'tests/catapult/utils/CountOfTests.cpp': 'catapult/types.h',
    'tests/catapult/utils/MacroBasedEnumTests.cpp': 'catapult/utils/MacroBasedEnumIncludes.h',
    'tests/catapult/utils/TraitsTests.cpp': 'catapult/utils/traits/Traits.h',
    'tests/catapult/utils/StlTraitsTests.cpp': 'catapult/utils/traits/StlTraits.h'
}

PLUGINS_FIRSTINCLUDES = {
    # plugins
    'plugins/coresystem/src/observers/PosImportanceCalculator.cpp': 'ImportanceCalculator.h',
    'plugins/coresystem/src/observers/RestoreImportanceCalculator.cpp': 'ImportanceCalculator.h',

    'plugins/coresystem/tests/observers/PosImportanceCalculatorTests.cpp': 'src/observers/ImportanceCalculator.h',
    'plugins/coresystem/tests/observers/RestoreImportanceCalculatorTests.cpp': 'src/observers/ImportanceCalculator.h',

    'plugins/txes/property/tests/model/PropertyTransactionTests.cpp': 'src/model/AddressPropertyTransaction.h',

    # sdk
    'sdk/tests/builders/AliasBuilderTests.cpp': 'src/builders/AddressAliasBuilder.h',
    'sdk/tests/builders/PropertyBuilderTests.cpp': 'src/builders/AddressPropertyBuilder.h',
}

TOOLS_FIRSTINCLUDES = {
    'tools/health/main.cpp': 'ApiNodeHealthUtils.h'
}

EXTENSION_FIRSTINCLUDES = {
    'extensions/mongo/plugins/mosaic/src/MongoMosaicPlugin.cpp': 'MosaicDefinitionMapper.h',
    'extensions/mongo/plugins/multisig/src/MongoMultisigPlugin.cpp': 'ModifyMultisigAccountMapper.h',
    'extensions/mongo/plugins/namespace/src/MongoNamespacePlugin.cpp': 'AddressAliasMapper.h',
}

SKIP_FORWARDS = (
    re.compile(r'src.catapult.validators.ValidatorTypes.h'),
    re.compile(r'src.catapult.utils.ClampedBaseValue.h'),
    re.compile(r'.*\.cpp$'),
)

FILTER_NAMESPACES = (
    re.compile(r'.*detail'),
    re.compile(r'.*_types::'),
    re.compile(r'.*_types$'),
    re.compile(r'.*bson_stream$')
)
