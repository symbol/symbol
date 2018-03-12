import re

# those files won't be checked at all
SKIP_FILES = (
    # macro-based enums
    re.compile('src.catapult.utils.MacroBasedEnum.h'),

    # inline includes
    re.compile('src.catapult.model.EntityType.cpp'),
    re.compile('src.catapult.validators.ValidationResult.cpp'),
    re.compile('tools.statusgen.main.cpp')
)

NAMESPACES_FALSEPOSITIVES = (
    # multiple namespaces (specialization)
    re.compile(r'src.catapult.utils.Logging.cpp'), # (boost::log)
    re.compile(r'tools.health.ApiNodeHealthUtils.cpp'), # (boost::asio)
    re.compile(r'tests.TestHarness.h'), # (std)

    # disallowed top-level namespaces
    re.compile(r'src.catapult.thread.detail.FutureSharedState.h'), # (detail)
    re.compile(r'tests.catapult.io.MemoryBasedStorageTests.cpp'), # (anon)

    # no types (only includes and/or fwd declares and/or defines)
    re.compile(r'src.catapult.constants.h'),
    re.compile(r'src.catapult.plugins.h'),
    re.compile(r'src.catapult.preprocessor.h'),
    re.compile(r'src.catapult.crypto.KeccakHash.h'),
    re.compile(r'src.catapult.utils.BitwiseEnum.h'),
    re.compile(r'src.catapult.utils.ExceptionLogging.h'),
    re.compile(r'src.catapult.utils.MacroBasedEnumIncludes.h'),
    re.compile(r'extensions.mongo.src.mappers.MapperInclude.h'),
    re.compile(r'extensions.mongo.src.CacheStorageInclude.h'),
    re.compile(r'plugins.txes.namespace.src.model.NamespaceConstants.h'),
    re.compile(r'tools.spammer.Generators.h'),
    re.compile(r'tests.test.nodeps.Stress.h'),

    # cache aliases (only headers without 'real' types)
    re.compile(r'plugins.services.hashcache.src.cache.HashCacheTypes.h'),
    re.compile(r'plugins.txes.multisig.src.cache.MultisigCacheTypes.h'),
    re.compile(r'plugins.txes.namespace.src.cache.MosaicCacheTypes.h'),
    re.compile(r'plugins.txes.namespace.src.cache.NamespaceCacheTypes.h'),

    # main entry points
    re.compile(r'src.catapult.server.main.cpp'),

    # mongo plugins (only entry point)
    re.compile(r'extensions.mongo.plugins.aggregate.src.MongoAggregatePlugin.cpp'),
    re.compile(r'extensions.mongo.plugins.lock.src.MongoLockPlugin.cpp'),
    re.compile(r'extensions.mongo.plugins.multisig.src.MongoMultisigPlugin.cpp'),
    re.compile(r'extensions.mongo.plugins.namespace.src.MongoNamespacePlugin.cpp'),
    re.compile(r'extensions.mongo.plugins.transfer.src.MongoTransferPlugin.cpp'),

    # everything in int tests, as there's no hierarchy there and we can't figure out ns
    re.compile(r'tests.int.*'),
)

EMPTYLINES_FALSEPOSITIVES = (
)

LONGLINES_FALSEPOSITIVES = (
    # 64-byte hex strings
    re.compile(r'Sha3Tests.cpp'),
    re.compile(r'SignerTests.cpp'),
)

SPECIAL_INCLUDES = (
    # src
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

    # tests
    re.compile(r'"BlockStorageTests.h"'),
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

    'src/catapult/ionet/IoEnums.cpp' : 'ConnectResult.h',
    'src/catapult/server/main.cpp' : 'ServerMain.h',

    # tests
    'tests/test/nodeps/TestMain.cpp': 'catapult/utils/Logging.h',

    'tests/catapult/consumers/AddressExtractionConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/BlockChainCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/BlockChainSyncConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/HashCalculatorConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/HashCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/NewBlockConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/NewTransactionsConsumerTests.cpp': 'catapult/consumers/TransactionConsumers.h',
    'tests/catapult/consumers/StatelessValidationConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',

    'tests/catapult/crypto/Sha3Tests.cpp': 'catapult/crypto/Hashes.h',
    'tests/catapult/deltaset/OrderedTests.cpp': 'tests/catapult/deltaset/test/BaseSetTestsInclude.h',
    'tests/catapult/deltaset/ReverseOrderedTests.cpp': 'tests/catapult/deltaset/test/BaseSetTestsInclude.h',
    'tests/catapult/deltaset/UnorderedMapTests.cpp': 'tests/catapult/deltaset/test/BaseSetTestsInclude.h',
    'tests/catapult/deltaset/UnorderedTests.cpp': 'tests/catapult/deltaset/test/BaseSetTestsInclude.h',
    'tests/catapult/io/MemoryBasedStorageTests.cpp': 'tests/test/core/mocks/MockMemoryBasedStorage.h',
    'tests/catapult/io/MemoryStreamTests.cpp': 'tests/test/core/mocks/MockMemoryStream.h',
    'tests/catapult/ionet/PacketPayloadBuilderTests.cpp': 'catapult/ionet/PacketPayload.h',
    'tests/catapult/thread/FutureSharedStateTests.cpp': 'catapult/thread/detail/FutureSharedState.h',
    'tests/catapult/utils/CatapultExceptionTests.cpp': 'catapult/exceptions.h',
    'tests/catapult/utils/CatapultTypesTests.cpp': 'catapult/types.h',
    'tests/catapult/utils/CountOfTests.cpp': 'catapult/types.h',
    'tests/catapult/utils/MacroBasedEnumTests.cpp': 'catapult/utils/MacroBasedEnumIncludes.h',
    'tests/catapult/utils/TraitsTests.cpp': 'catapult/utils/traits/Traits.h',

    # clang workaround
    'tests/catapult/utils/StackLoggerTests.cpp': '<string>',
    'tests/catapult/utils/test/LoggingTestUtils.cpp': '<string>',
    'tests/test/nodeps/Filesystem.cpp': '<string>',
}

PLUGINS_FIRSTINCLUDES = {
    'plugins/coresystem/src/observers/PosImportanceCalculator.cpp': 'ImportanceCalculator.h',
    'plugins/coresystem/src/observers/RestoreImportanceCalculator.cpp': 'ImportanceCalculator.h',

    'plugins/coresystem/tests/observers/PosImportanceCalculatorTests.cpp': 'src/observers/ImportanceCalculator.h',
    'plugins/coresystem/tests/observers/RestoreImportanceCalculatorTests.cpp': 'src/observers/ImportanceCalculator.h',

    'plugins/txes/lock/tests/cache/LockInfoCacheTests.cpp': 'src/cache/HashLockInfoCache.h',
}

TOOLS_FIRSTINCLUDES = {
    'tools/health/main.cpp': 'ApiNodeHealthUtils.h',
    'tools/nemgen/main.cpp': 'NemesisConfiguration.h',
    'tools/spammer/main.cpp': 'Generators.h'
}

EXTENSION_FIRSTINCLUDES = {
    # mongo
    'extensions/mongo/plugins/aggregate/src/MongoAggregatePlugin.cpp': 'AggregateMapper.h',
    'extensions/mongo/plugins/aggregate/tests/MongoAggregatePluginTests.cpp': 'mongo/tests/test/MongoPluginTestUtils.h',
    'extensions/mongo/plugins/lock/src/MongoLockPlugin.cpp': 'HashLockMapper.h',
    'extensions/mongo/plugins/lock/tests/MongoLockPluginTests.cpp': 'mongo/tests/test/MongoPluginTestUtils.h',
    'extensions/mongo/plugins/multisig/src/MongoMultisigPlugin.cpp': 'ModifyMultisigAccountMapper.h',
    'extensions/mongo/plugins/multisig/tests/MongoMultisigPluginTests.cpp': 'mongo/tests/test/MongoPluginTestUtils.h',
    'extensions/mongo/plugins/namespace/src/MongoNamespacePlugin.cpp': 'MosaicDefinitionMapper.h',
    'extensions/mongo/plugins/namespace/tests/MongoNamespacePluginTests.cpp': 'mongo/tests/test/MongoPluginTestUtils.h',
    'extensions/mongo/plugins/transfer/src/MongoTransferPlugin.cpp': 'TransferMapper.h',
    'extensions/mongo/plugins/transfer/tests/MongoTransferPluginTests.cpp': 'mongo/tests/test/MongoPluginTestUtils.h',
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
