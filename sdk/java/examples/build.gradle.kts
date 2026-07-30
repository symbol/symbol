// Runnable Java ports of `sdk/javascript/examples/*.js`. Each example is its own `main(String[])`
// class under `org.symbol.examples`, wired here as a Gradle `JavaExec` task. Mirrors the
// `npm run examples` / `python3 -m examples.*` invocation pattern so `scripts/ci/test_examples.sh`
// can run them one at a time.
plugins {
	`java`
	jacoco
	id("com.diffplug.spotless") version "6.25.0"
}

java {
	toolchain {
		languageVersion.set(JavaLanguageVersion.of(21))
	}
}

repositories {
	mavenCentral()
}

dependencies {
	implementation(rootProject)
}

spotless {
	java {
		target("src/**/*.java")
		trimTrailingWhitespace()
		endWithNewline()
		removeUnusedImports()
		importOrder("java", "javax", "org.bouncycastle", "", "org.symbol")
	}
}

// Map of Gradle task name -> (main class, default args). Tracks the set the JS / Python
// `test_examples.sh` scripts invoke; `transaction_aggregate` mirrors the JS `--private` flag.
val exampleSpecs: Map<String, Pair<String, List<String>>> = linkedMapOf(
	"runBip32Keypair" to ("org.symbol.examples.Bip32Keypair" to emptyList()),
	"runTransactionAggregate" to (
		"org.symbol.examples.TransactionAggregate"
			to listOf("--private", "src/main/resources/zero.sha256.txt")
	),
	"runTransactionMultisig" to ("org.symbol.examples.TransactionMultisig" to emptyList()),
	"runTransactionSignNem" to (
		"org.symbol.examples.TransactionSign" to listOf("--blockchain=nem")
	),
	"runTransactionSignSymbol" to (
		"org.symbol.examples.TransactionSign" to listOf("--blockchain=symbol")
	),
	"runReadmeNem" to ("org.symbol.examples.readme.Nem" to emptyList()),
	"runReadmeSymbol" to ("org.symbol.examples.readme.Symbol" to emptyList())
)

exampleSpecs.forEach { (taskName, spec) ->
	tasks.register<JavaExec>(taskName) {
		group = "examples"
		description = "Runs ${spec.first}"
		classpath = sourceSets["main"].runtimeClasspath
		mainClass.set(spec.first)
		args = spec.second
	}
}

// JaCoCo only auto-attaches its task extension to Test tasks; for JavaExec we have to apply
// it explicitly. Each example writes its coverage data into the root project's jacoco
// directory so the root jacocoTestReport task picks it up via its fileTree merge.
jacoco.applyTo(tasks.withType<JavaExec>())
val coverageEnabled = providers.gradleProperty("coverage").isPresent
exampleSpecs.keys.forEach { taskName ->
	tasks.named<JavaExec>(taskName).configure {
		extensions.configure<JacocoTaskExtension> {
			isEnabled = coverageEnabled
			destinationFile = rootProject.layout.buildDirectory.file("jacoco/example-${taskName}.exec").get().asFile
		}
	}
}
