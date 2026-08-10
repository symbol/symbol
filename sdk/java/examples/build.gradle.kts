// Runnable Java ports of `sdk/javascript/examples/*.js`. Each example is its own `main(String[])`
// class under `org.symbol.examples`, wired here as a Gradle `JavaExec` task. Mirrors the
// `npm run examples` / `python3 -m examples.*` invocation pattern so `scripts/ci/test_examples.sh`
// can run them one at a time.
plugins {
	`java`
	jacoco
	id("com.diffplug.spotless") version "8.9.0"
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

// Map of Gradle task name -> main class. Example arguments come from the command line (`--args="..."`)
// see `scripts/ci/test_examples.sh` for the flags each example expects.
val exampleSpecs: Map<String, String> = linkedMapOf(
	"runBip32Keypair" to "org.symbol.examples.Bip32Keypair",
	"runTransactionAggregate" to "org.symbol.examples.TransactionAggregate",
	"runTransactionMultisig" to "org.symbol.examples.TransactionMultisig",
	"runTransactionSign" to "org.symbol.examples.TransactionSign",
	"runReadmeNem" to "org.symbol.examples.readme.Nem",
	"runReadmeSymbol" to "org.symbol.examples.readme.Symbol"
)

exampleSpecs.forEach { (taskName, mainClassName) ->
	tasks.register<JavaExec>(taskName) {
		group = "examples"
		description = "Runs $mainClassName"
		classpath = sourceSets["main"].runtimeClasspath
		mainClass.set(mainClassName)
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
