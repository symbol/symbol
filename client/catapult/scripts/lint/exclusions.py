import re

#those files won't be checked at all
SKIP_FILES = (
    re.compile('src.catapult.server.api.main.cpp'),
    re.compile('src.catapult.server.p2p.main.cpp'),
    re.compile('src.catapult.crypto.KeccakHash.h'),
    re.compile('src.catapult.ionet.IoEnums.cpp'), # basicaly collection of different enums, there's on IoEnums.h
    re.compile('src.catapult.utils.MacroBasedEnum.h'),
    re.compile('src.catapult.validators.ValidationResult.cpp'),

    re.compile('tests.catapult.deltaset.UnorderedMapTests.cpp'),
    re.compile('tests.catapult.validators.SequentialValidationPolicyTests.cpp'),
    re.compile('tests.catapult.utils.MacroBasedEnumTests.cpp'),
    re.compile('tests.test.nodeps.TestMain.cpp'),
)

NAMESPACES_FALSEPOSITIVES = (
    re.compile(r'src.catapult.utils.Logging.cpp'),
    re.compile(r'src.catapult.utils.Traits.h'),
    re.compile(r'src.catapult.constants.h'),
    re.compile(r'src.catapult.plugins.h'),
    re.compile(r'src.catapult.preprocessor.h'),
    re.compile(r'src.catapult.thread.FutureSharedState.h'),
    re.compile(r'src.catapult.utils.MacroBasedEnumIncludes.h'),

    # dummy includes
    re.compile(r'src.catapult.cache.AccountStateCache.h'),
    re.compile(r'src.catapult.cache.BlockDifficultyCache.h'),
    re.compile(r'src.catapult.cache.ImportanceView.h'),

    re.compile(r'tests.catapult.api.RemoteChainApiTests.cpp'),
    re.compile(r'tests.catapult.api.RemoteRequestDispatcherTests.cpp'),
    re.compile(r'tests.catapult.api.RemoteTransactionApiTests.cpp'),
    re.compile(r'tests.catapult.cache.CatapultCacheTests.cpp'),            # multiple namespaces (catapult::test specialization)
    re.compile(r'tests.catapult.cache.SubCachePluginAdapterTests.cpp'),    # multiple namespaces (catapult::test specialization)
    re.compile(r'tests.catapult.io.MemoryBasedStorageTests.cpp'),
    re.compile(r'tests.test.nodeps.Stress.h'),

    # plugins
    re.compile(r'plugins.coresystem.src.cache.AccountStateCacheTypes.h'),
    re.compile(r'plugins.mongo.multisig.src.MongoMultisigPlugin.cpp'),
    re.compile(r'plugins.mongo.namespace.src.MongoNamespacePlugin.cpp'),
    re.compile(r'plugins.services.hashcache.src.cache.HashCacheTypes.h'),
    re.compile(r'plugins.txes.multisig.src.cache.MultisigCacheTypes.h'),
    re.compile(r'plugins.txes.namespace.src.cache.MosaicCacheTypes.h'),
    re.compile(r'plugins.txes.namespace.src.cache.NamespaceCacheTypes.h'),

    # dummy includes (plugins)
    re.compile(r'plugins.txes.namespace.src.model.NamespaceConstants.h'),
    re.compile(r'plugins.mongo.coremongo.src.MongoDatabasePlugin.h'),

    # everything in int tests, as there's no hierarchy there and we can't figure out ns
    re.compile(r'tests.int.*'),

    # tools
    re.compile(r'tools.*.main.cpp'),
    re.compile(r'tools.spammer.Generators.h'),
)

EMPTYLINES_FALSEPOSITIVES = (
    re.compile(r'src.catapult.preprocessor.h'),
    re.compile(r'tests.test.cache.CacheContentsTests.h'),
)

LONGLINES_FALSEPOSITIVES = (
    re.compile(r'Sha3Tests.cpp'),                       # patterns
    re.compile(r'SignerTests.cpp'),                     # patterns
    re.compile(r'ReadOnlyCatapultCacheTests.cpp'),      # long macro
    re.compile(r'PacketEntityUtilsTests.cpp'),          # long macro
    re.compile(r'FutureSharedStateTests.cpp'),          # long macro
    re.compile(r'FutureTests.cpp'),                     # long macro
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
    re.compile(r'"tests/catapult/validators/utils/ValidationPolicyTests.h"'),
    re.compile(r'"BlockStorageTests.h"'),
)

CORE_FIRSTINCLUDES = {
    # src
    'src/catapult/consumers/BlockChainCheckConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/BlockChainSyncConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/HashCalculatorConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/HashCheckConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/NewBlockConsumer.cpp': 'BlockConsumers.h',
    'src/catapult/consumers/StatelessValidationConsumer.cpp': 'BlockConsumers.h',

    'src/catapult/consumers/NewTransactionsConsumer.cpp': 'TransactionConsumers.h',

    # tests
    'tests/catapult/consumers/BlockChainCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/BlockChainSyncConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/HashCalculatorConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/HashCheckConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/NewBlockConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',
    'tests/catapult/consumers/NewTransactionsConsumerTests.cpp': 'catapult/consumers/TransactionConsumers.h',
    'tests/catapult/consumers/StatelessValidationConsumerTests.cpp': 'catapult/consumers/BlockConsumers.h',

    'tests/catapult/crypto/Sha3Tests.cpp': 'catapult/crypto/Hashes.h',
    'tests/catapult/deltaset/OrderedTests.cpp': 'tests/catapult/deltaset/utils/BaseSetTestsInclude.h',
    'tests/catapult/deltaset/ReverseOrderedTests.cpp': 'tests/catapult/deltaset/utils/BaseSetTestsInclude.h',
    'tests/catapult/deltaset/UnorderedTests.cpp': 'tests/catapult/deltaset/utils/BaseSetTestsInclude.h',
    'tests/catapult/io/MemoryBasedStorageTests.cpp': 'tests/test/core/mocks/MemoryBasedStorage.h',
    'tests/catapult/io/MemoryStreamTests.cpp': 'tests/test/core/mocks/MemoryStream.h',
    'tests/catapult/utils/CatapultExceptionTests.cpp': 'catapult/exceptions.h',
    'tests/catapult/utils/CatapultTypesTests.cpp': 'catapult/types.h',
    'tests/catapult/utils/CountOfTests.cpp': 'catapult/types.h',
    'tests/test/core/mocks/MemoryBasedStorage_data.cpp': 'MemoryBasedStorage.h',

    # clang workaround
    'tests/catapult/utils/StackLoggerTests.cpp': '<string>',
    'tests/catapult/utils/utils/LoggingTestUtils.cpp': '<string>',
    'tests/test/nodeps/Filesystem.cpp': '<string>',
}

PLUGINS_FIRSTINCLUDES = {
    # plugins src
    'plugins/coresystem/src/observers/PosImportanceCalculator.cpp': 'ImportanceCalculator.h',
    'plugins/coresystem/src/observers/RestoreImportanceCalculator.cpp': 'ImportanceCalculator.h',
    'plugins/mongo/multisig/src/MongoMultisigPlugin.cpp': 'ModifyMultisigAccountMapper.h',
    'plugins/mongo/namespace/src/MongoNamespacePlugin.cpp': 'MosaicDefinitionMapper.h',

    'plugins/coresystem/tests/observers/PosImportanceCalculatorTests.cpp': 'src/observers/ImportanceCalculator.h',
    'plugins/coresystem/tests/observers/RestoreImportanceCalculatorTests.cpp': 'src/observers/ImportanceCalculator.h',
}

TOOLS_FIRSTINCLUDES = {
    'tools/uts/main.cpp': 'tools/ToolMain.h',
    'tools/spammer/main.cpp': 'Generators.h',
    'tools/nemgen/main.cpp': 'NemesisConfiguration.h',
    'tools/nsgen/main.cpp': 'tools/ToolMain.h',
    'tools/health/main.cpp': 'tools/ToolMain.h',
    'tools/benchmark/main.cpp': 'tools/ToolMain.h',
}

SKIP_FORWARDS = (
    re.compile(r'src.catapult.validators.ValidatorTypes.h'),
    re.compile(r'src.catapult.utils.ClampedBaseValue.h'),
    re.compile(r'.*\.cpp$'),
)
